#include "shared.h"

#include <unistd.h>
#include <iostream>
#ifdef _WIN32
#else
#include <csignal>
#include <sys/wait.h>
#include <cstring>
#endif

#include "ren-general/arrangement.h"

InteractionError::InteractionError(String const &Message) : Message(Message) {}
ControllerError::ControllerError(String const &Message) : Message(Message) {}

std::queue<String> SplitString(String const &Input, Set<char> const &Delimiters, bool DropBlanks)
{
	std::queue<String> Out;

	char const Slash = '\\';
	char const Quote = '"';
	bool HotSlash = false, HotQuote = false;
	unsigned int WordStart = 0;
	for (unsigned int CharacterIndex = 0; CharacterIndex < Input.length(); ++CharacterIndex)
	{
		char const &CurrentCharacter = Input[CharacterIndex];
		if (CurrentCharacter == Slash)
		{
			HotSlash = !HotSlash;
			continue;
		}

		if (HotSlash)
		{
			HotSlash = false;
			continue;
		}

		if (CurrentCharacter == Quote)
		{
			HotQuote = !HotQuote;
			continue;
		}

		if (!HotQuote && Delimiters.Contains(CurrentCharacter))
		{
			if (!DropBlanks || (CharacterIndex - WordStart > 1))
				Out.push(Input.substr(WordStart, CharacterIndex - WordStart));
			WordStart = CharacterIndex + 1;
		}
	}

	if (!DropBlanks || (Input.length() - WordStart > 1))
		Out.push(Input.substr(WordStart, Input.length() - WordStart));

	return std::move(Out);
}

String GetNextArgument(std::queue<String> &Arguments, String const &Name)
{
	if (Arguments.empty())
		throw ControllerError("Request ended before providing " + Name);
	String NextArgument = Arguments.front();
	Arguments.pop();
	return std::move(NextArgument);
}

extern Set<String> ProgramArguments;

std::pair<bool, String> FindProgramArgument(String const &Name)
{
	bool Found = false;
	Set<String>::iterator ArgumentFound = ProgramArguments.find(Name);
	String Value;
	if (ArgumentFound != ProgramArguments.end())
		Found = true;
	else if ((ArgumentFound = ProgramArguments.lower_bound(Name)) != ProgramArguments.end())
	{
		size_t EqualFound = ArgumentFound->find('=');
		String Front = *ArgumentFound;
		if (EqualFound != String::npos)
			Front = ArgumentFound->substr(0, EqualFound);
		if (Front == Name)
		{
			Found = true;
			Value = ArgumentFound->substr(EqualFound + 1, String::npos);
		}
	}

	return std::pair<bool, String>(Found, Value);
}

namespace Information
{
	Anchor::~Anchor(void) {}
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

	if (!Contents.empty())
	{
		int Wrote = write(FileDescriptor, Contents.c_str(), Contents.length());
		if (Wrote == -1) throw WriteError();
	}
	char const NewLine[] = "\n";
	int Wrote = write(FileDescriptor, NewLine, sizeof(NewLine) - 1);
	if (Wrote == -1) throw WriteError();
}

Subprocess::Subprocess(FilePath const &Execute, std::vector<String> const &Arguments) : ResultRetrieved(false)
{
#ifdef WINDOWS
	// TODO
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

void Subprocess::Kill(void) { kill(ChildID, SIGKILL); }

int Subprocess::GetResult(void)
{
	if (!ResultRetrieved) 
	{
		int RawStatus;
		waitpid(ChildID, &RawStatus, 0);
		if (!WIFEXITED(RawStatus))
			return 1; // Some error, pretend like command failed
		Result = WEXITSTATUS(RawStatus);
		ResultRetrieved = true;
	}
	return Result;
}

