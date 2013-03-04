#include "subprocess.h"

#include <unistd.h>
#include <iostream>
#include <cstring>
#include <cassert>
#ifdef _WIN32
#include <fcntl.h>
#else
#include <csignal>
#include <sys/wait.h>
#endif

#include "shared.h"

extern bool Verbose;

SubprocessOutStream::SubprocessOutStream(void) : FileDescriptor(-1), Failed(true) {}

SubprocessOutStream::~SubprocessOutStream(void) { /*if (FileDescriptor >= 0) close(FileDescriptor);*/ }

void SubprocessOutStream::Associate(int FileDescriptor)
{
	assert(this->FileDescriptor == -1);
	assert(FileDescriptor != -1);
	this->FileDescriptor = FileDescriptor;
	Failed = false;
}

String SubprocessOutStream::ReadLine(void)
{
	assert(!Failed);
	String Out;
	while (true)
	{
		char Buffer;
		int Result = read(FileDescriptor, &Buffer, 1);
		if (Result == -1)
		{
			std::cerr << "Error: Couldn't read from subprocess due to error " << errno << ": " << strerror(errno) << std::endl;
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

bool SubprocessOutStream::HasFailed(void) { return Failed; }
		
void SubprocessOutStream::ReadToEnd(void)
{
	while (!HasFailed())
		ReadLine();
}

SubprocessInStream::SubprocessInStream(void) : FileDescriptor(-1) {}

SubprocessInStream::~SubprocessInStream(void) { /*if (FileDescriptor >= 0) close(FileDescriptor);*/ }

void SubprocessInStream::Associate(int FileDescriptor)
{
	assert(this->FileDescriptor == -1);
	this->FileDescriptor = FileDescriptor;
}

void SubprocessInStream::Write(String const &Contents)
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
	if (Verbose)
	{
		StandardStream << "Running \"" << Execute << "\" with arguments: ";
		for (auto &Argument : Arguments) StandardStream << "\"" << Argument << "\" ";
		StandardStream << "\n" << OutputStream::Flush();
	}

	fflush(nullptr); // Write everything before forking so buffered data isn't written twice
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
	ChildStartupInformation.hStdOutput = ChildOutHandle;
	ChildStartupInformation.hStdInput = ChildInHandle;
	if (Verbose)
		ChildStartupInformation.hStdError = ChildOutHandle;
		//ChildStartupInformation.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	ChildStartupInformation.dwFlags |= STARTF_USESTDHANDLES;	
 
	MemoryStream ArgumentConcatenation; 
	ArgumentConcatenation << Execute.AsAbsoluteString();
	for (auto &Argument : Arguments) ArgumentConcatenation << " \"" << Argument << "\"";
	NativeString NativeArguments = AsNativeString(ArgumentConcatenation);
	std::vector<wchar_t> NativeArgumentsWritableBuffer;
	NativeArgumentsWritableBuffer.resize(NativeArguments.length() + 1);
	std::copy(NativeArguments.begin(), NativeArguments.end(), NativeArgumentsWritableBuffer.begin());
	NativeArgumentsWritableBuffer[NativeArguments.length()] = 0;
	
	memset(&ChildStatus, 0, sizeof(PROCESS_INFORMATION));
	
	bool Result = CreateProcessW(reinterpret_cast<wchar_t const *>(AsNativeString(Execute).c_str()), &NativeArgumentsWritableBuffer[0], nullptr, nullptr, true, 0, nullptr, nullptr, &ChildStartupInformation, &ChildStatus);
	if (!Result) throw InteractionError("Failed to spawn child process with name '" + (String)Execute + "' and arguments '" + (String)ArgumentConcatenation + ": error number " + AsString(GetLastError()));

	CloseHandle(ChildStatus.hThread);
	CloseHandle(ChildOutHandle);
	CloseHandle(ChildInHandle);
	
	int ParentIn = _open_osfhandle((intptr_t)ParentInHandle, _O_RDONLY);
	if (ParentIn == -1) throw InteractionError("Failed to get a file descriptor for parent read pipe.");
	
	int ParentOut = _open_osfhandle((intptr_t)ParentOutHandle, _O_APPEND);
	if (ParentOut == -1) throw InteractionError("Failed to get a file descriptor for parent write pipe.");
	
	Out.Associate(ParentIn);
	In.Associate(ParentOut);
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
		MemoryStream FullRunLine;
		FullRunLine << Execute.AsAbsoluteString();
		for (auto &Argument : Arguments)
			FullRunLine << " " << Argument;
		int RawResult = system(((String)FullRunLine).c_str());
		if (RawResult == -1) exit(1);
		exit(WEXITSTATUS(RawResult));
	}
	else // Parent side
	{
		close(ToChild[ReadEnd]);
		In.Associate(ToChild[WriteEnd]);

		close(FromChild[WriteEnd]);
		Out.Associate(FromChild[ReadEnd]);
	}
#endif
}

Subprocess::~Subprocess(void) 
{
#ifdef WINDOWS
	CloseHandle(ChildStatus.hProcess);
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
		DWORD ReturnCode = 0;
		if (!GetExitCodeProcess(ChildStatus.hProcess, &ReturnCode))
			throw InteractionError("Lost control of child process, can't get return value: error code " + AsString(GetLastError()));
		Result = ReturnCode;
#else
		int RawStatus = 1;
		waitpid(ChildID, &RawStatus, 0);
		if (!WIFEXITED(RawStatus))
			return 1; // Some error, pretend like command failed
		Result = WEXITSTATUS(RawStatus);
#endif
		if (Verbose) StandardStream << "Execution finished with code " << Result << ".\n" << OutputStream::Flush();
		ResultRetrieved = true;
	}
	return Result;
}

