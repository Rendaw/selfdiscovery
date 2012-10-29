#include "shared.h"

#include <unistd.h>
#include <iostream>
#include <cstring>
#ifdef _WIN32
#include <fcntl.h>
#else
#include <csignal>
#include <sys/wait.h>
#endif

#include "ren-general/arrangement.h"

InteractionError::InteractionError(String const &Message) : Message(Message) {}
ControllerError::ControllerError(String const &Message) : Message(Message) {}

StringSplitter::StringSplitter(Set<char> const &Delimiters, bool DropBlanks) : 
	Delimiters(Delimiters), DropBlanks(DropBlanks) HotSlash(false), HotQuote(false) {}

StringSplitter &StringSplitter::Process(String const &Input)
{
	String Buffer;
	Buffer.reserve(1000);

	char const Slash = '\\';
	char const Quote = '"';
	for (unsigned int CharacterIndex = 0; CharacterIndex < Input.length(); ++CharacterIndex)
	{
		char const &CurrentCharacter = Input[CharacterIndex];
		if (!HotSlash)
		{
			if (CurrentCharacter == Slash)
			{
				HotSlash = true;
				continue;
			}
		
			if (CurrentCharacter == Quote)
			{
				HotQuote = !HotQuote;
				continue;
			}
		}

		if (HotSlash)
		{
			HotSlash = false;
		}

		if (!HotQuote && Delimiters.Contains(CurrentCharacter))
		{
			if (!DropBlanks || !Buffer.empty())
				Out.push(Buffer);
			Buffer.clear();
			continue;
		}
		
		Buffer.push_back(CurrentCharacter);
	}

	if (!DropBlanks || !Buffer.empty())
		Out.push(Buffer);

	return *this;
}

bool StringSplitter::Unfinished(void) { return HotSlash || HotQuote; }

std::queue<String> &StringSplitter::Results(void) { return Out; }

String GetNextArgument(std::queue<String> &Arguments, String const &Name)
{
	if (Arguments.empty())
		throw ControllerError("Request ended before providing " + Name);
	String NextArgument = Arguments.front();
	Arguments.pop();
	return std::move(NextArgument);
}

SubprocessInStream::SubprocessInStream(void) : FileDescriptor(-1), Failed(true) {}

SubprocessInStream::~SubprocessInStream(void) { if (FileDescriptor >= 0) close(FileDescriptor); }

void SubprocessInStream::Associate(int FileDescriptor)
{
	assert(this->FileDescriptor == -1);
	this->FileDescriptor = FileDescriptor;
	Failed = false;
}

String SubprocessInStream::ReadLine(void)
{
	assert(!Failed);
	String Out;
	while (true)
	{
		char Buffer;
		int Result = read(FileDescriptor, &Buffer, 1);
		if (Result == -1)
		{
			std::cerr << "Error: Couldn't read from controller due to error " << errno << ": " << strerror(errno) << std::endl;
			Failed = true;
			return Out;
		}
		if (Result == 0)
		{
			Failed = true;
			return Out;
		}
		if (Buffer == '\r') continue;
		if (Buffer == '\n') break;
		Out += Buffer;
	}

	return std::move(Out);
}

bool SubprocessInStream::HasFailed(void) { return Failed; }
		
void SubprocessInStream::ReadToEnd(void)
{
	while (!HasFailed())
		ReadLine();
}

SubprocessOutStream::SubprocessOutStream(void) : FileDescriptor(-1) {}

SubprocessOutStream::~SubprocessOutStream(void) { if (FileDescriptor >= 0) close(FileDescriptor); }

void SubprocessOutStream::Associate(int FileDescriptor)
{
	assert(this->FileDescriptor == -1);
	this->FileDescriptor = FileDescriptor;
}

void SubprocessOutStream::Write(String const &Contents)
{
	struct WriteError : public InteractionError
	{
		WriteError(void) :
			InteractionError("Couldn't write to controller due to error " + AsString(errno) + ": " + strerror(errno))
			{}
	};

	assert(!Contents.empty());
	int Wrote = write(FileDescriptor, Contents.c_str(), Contents.length());
	if (Wrote == -1) throw WriteError();
}

Subprocess::Subprocess(FilePath const &Execute, std::vector<String> const &Arguments) : ResultRetrieved(false)
{
#ifdef WINDOWS
	HANDLE ChildInHandle = NULL;
	HANDLE ParentOutHandle = NULL;
	HANDLE ParentInHandle = NULL;
	HANDLE ChildOutHandle = NULL;

	SECURITY_ATTRIBUTES SecurityAttributes; 
	SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES); 
	SecurityAttributes.bInheritHandle = TRUE; 
	SecurityAttributes.lpSecurityDescriptor = NULL; 

	if (!CreatePipe(&ParentInHandle, &ChildOutHandle, &SecurityAttributes, 0)) 
		throw InteractionError("Failed to create parent read pipe: error number " + AsString(GetLastError()));

	if (!SetHandleInformation(ParentInHandle, HANDLE_FLAG_INHERIT, 0))
		throw InteractionError("Failed to make parent read pipe uninheritable: error number " + AsString(GetLastError()));

	if (!CreatePipe(&ChildInHandle, &ParentOutHandle, &SecurityAttributes, 0)) 
		throw InteractionError("Failed to create parent write pipe: error number " + AsString(GetLastError()));

	if (!SetHandleInformation(ParentOutHandle, HANDLE_FLAG_INHERIT, 0))
		throw InteractionError("Failed to make parent write pipe uninheritable: error number " + AsString(GetLastError()));
	
	STARTUPINFOW ChildStartupInformation;
	memset(&ChildStartupInformation, 0, sizeof(STARTUPINFO));
	ChildStartupInformation.cb = sizeof(STARTUPINFO); 
	//ChildStartupInformation.hStdError = g_hChildStd_OUT_Wr;
	ChildStartupInformation.hStdOutput = ChildOutHandle;
	ChildStartupInformation.hStdInput = ChildInHandle;
	ChildStartupInformation.dwFlags |= STARTF_USESTDHANDLES;	
 
	StringStream ArgumentConcatenation; 
#ifdef __MINGW32__
	ArgumentConcatenation << "-c";
#else
	ArgumentConcatenation << "/c";
#endif
	for (auto &Argument : Arguments) ArgumentConcatenation << " " << Argument;
	NativeString NativeArguments = AsNativeString(ArgumentConcatenation.str());
	std::vector<wchar_t> NativeArgumentsWritableBuffer;
	NativeArgumentsWritableBuffer.resize(NativeArguments.length());
	std::copy(NativeArguments.begin(), NativeArguments.end(), NativeArgumentsWritableBuffer.begin());
	
	memset(&ChildStatus, 0, sizeof(PROCESS_INFORMATION));
	
#ifdef __MINGW32__
	const String ProcessName = "sh.exe";
#else
	const String ProcessName = "cmd.exe";
#endif
	bool Result = CreateProcessW(reinterpret_cast<wchar_t const *>(AsNativeString(ProcessName).c_str()), &NativeArgumentsWritableBuffer[0], nullptr, nullptr, true, 0, nullptr, nullptr, &ChildStartupInformation, &ChildStatus);
	if (!Result) throw InteractionError("Failed to spawn child process with name '" + ProcessName + "' and arguments '" + ArgumentConcatenation.str() + ": error number " + AsString(GetLastError()));
	
	CloseHandle(ChildStatus.hProcess);
	CloseHandle(ChildStatus.hThread);
	
	int ParentIn = _open_osfhandle((intptr_t)ParentInHandle, _O_RDONLY);
	if (ParentIn == -1) throw InteractionError("Failed to get a file descriptor for parent read pipe.");
	
	int ParentOut = _open_osfhandle((intptr_t)ParentOutHandle, _O_APPEND);
	if (ParentOut == -1) throw InteractionError("Failed to get a file descriptor for parent write pipe.");
	
	In.Associate(ParentIn);
	Out.Associate(ParentOut);
	
#else
	const unsigned int WriteEnd = 1, ReadEnd = 0;
	int FromChild[2], ToChild[2];
	if ((pipe(FromChild) == -1) || (pipe(ToChild) == -1))
		throw InteractionError("Error: Failed to create pipes for communication with controller.");

	ChildID = fork();
	if (ChildID == -1) throw InteractionError("Failed to create process for controller.");

	if (ChildID == 0) // Child side
	{
		close(ToChild[WriteEnd]);
		dup2(ToChild[ReadEnd], 0);
		close(ToChild[ReadEnd]);
		close(FromChild[ReadEnd]);
		dup2(FromChild[WriteEnd], 1);
		close(FromChild[WriteEnd]);
		StringStream FullRunLine;
		FullRunLine << Execute.AsAbsoluteString();
		for (auto &Argument : Arguments)
			FullRunLine << " " << Argument;
		exit(system(FullRunLine.str().c_str()));
	}
	else // Parent side
	{
		close(ToChild[ReadEnd]);
		Out.Associate(ToChild[WriteEnd]);

		close(FromChild[WriteEnd]);
		In.Associate(FromChild[ReadEnd]);
	}
#endif
}

void Subprocess::Kill(void) 
{
#ifdef WINDOWS
	TerminateProcess(ChildStatus.hProcess, 1);
#else
	kill(ChildID, SIGKILL); 
#endif
}

int Subprocess::GetResult(void)
{
	if (!ResultRetrieved) 
	{
#ifdef WINDOWS
		WaitForSingleObject(ChildStatus.hProcess, 0);
		DWORD ReturnCode;
		if (!GetExitCodeProcess(ChildStatus.hProcess, &ReturnCode))
			throw InteractionError("Lost control of child process, can't get return value: error code " + AsString(GetLastError()));
		Result = ReturnCode;
#else
		int RawStatus;
		waitpid(ChildID, &RawStatus, 0);
		if (!WIFEXITED(RawStatus))
			return 1; // Some error, pretend like command failed
		Result = WEXITSTATUS(RawStatus);
#endif
		ResultRetrieved = true;
	}
	return Result;
}

