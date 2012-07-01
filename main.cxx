// TODO Rewrite help to take arguments - change flag to use first argument in help instead of NAME.
// TODO Handle value overloading for platform and location (depend on platform).

#include <iostream>
#include <cstring>
#include <queue>
#include <unistd.h>

#include "ren-general/string.h"
#include "ren-general/arrangement.h"
#include "ren-general/lifetime.h"

#include "shared.h"

// Global information and information types - used in main loop and in individual info types and such
Set<String> ProgramArguments;
bool HelpMode = false;
bool Verbose = false;

#include "blank.h"
#include "flag.h"
#include "platform.h"
#include "location.h"

class ChildInStream
{
	public:
		ChildInStream(void) : FileDescriptor(-1), Failed(true) {}

		~ChildInStream(void) { assert(FileDescriptor >= 0); close(FileDescriptor); }

		void Associate(int FileDescriptor)
		{
			assert(this->FileDescriptor == -1);
			this->FileDescriptor = FileDescriptor;
			Failed = false;
		}

		String ReadLine(void)
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

		bool HasFailed(void) { return Failed; }

	private:
		int FileDescriptor;
		bool Failed;
};

class ChildOutStream
{
	public:
		ChildOutStream(void) : FileDescriptor(-1) {}

		~ChildOutStream(void) { assert(FileDescriptor >= 0); close(FileDescriptor); }

		void Associate(int FileDescriptor)
		{
			assert(this->FileDescriptor == -1);
			this->FileDescriptor = FileDescriptor;
		}

		void Write(String const &Contents = String())
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
			int Wrote = write(FileDescriptor, "\n", 1);
			if (Wrote == -1) throw WriteError();
		}

	private:
		int FileDescriptor;
};

void ForkController(String const &ControllerCommand, ChildInStream &InStream, ChildOutStream &OutStream)
{
#ifdef WINDOWS
	// TODO
#else
	const unsigned int WriteEnd = 1, ReadEnd = 0;
	int FromChild[2], ToChild[2];
	if ((pipe(FromChild) == -1) || (pipe(ToChild) == -1))
		throw InteractionError("Error: Failed to create pipes for communication with controller.");

	pid_t Result = fork();
	if (Result == -1) throw InteractionError("Failed to create process for controller.");

	if (Result == 0) // Child side
	{
		close(ToChild[WriteEnd]);
		dup2(ToChild[ReadEnd], 0);
		close(ToChild[ReadEnd]);
		close(FromChild[ReadEnd]);
		dup2(FromChild[WriteEnd], 1);
		close(FromChild[WriteEnd]);
		exit(system(ControllerCommand.c_str()));
	}
	else // Parent side
	{
		close(ToChild[ReadEnd]);
		InStream.Associate(ToChild[WriteEnd]);

		close(FromChild[WriteEnd]);
		OutStream.Associate(FromChild[ReadEnd]);
	}
#endif
}

std::queue<String> ParseInput(String const &Input)
{
	std::queue<String> Out;

	Set<char> const Whitespace = {' ', '\t', '\n', '\r'};
	char const Slash = '\\';
	char const Quote = '"';
	bool HotSlash = false, HotQuote = false;
	unsigned int WordStart = 0;
	for (unsigned int CharacterIndex = 0; CharacterIndex < Input.length(); CharacterIndex++)
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

		if (!HotQuote && Whitespace.Contains(CurrentCharacter))
		{
			if (CharacterIndex - WordStart > 1)
				Out.push(Input.substr(WordStart, CharacterIndex));
			WordStart = CharacterIndex;
		}
	}

	if (Input.length() - WordStart > 1)
		Out.push(Input.substr(WordStart, Input.length()));

	return std::move(Out);
}

int main(int argc, char **argv)
{
	try {
		// Process arguments, store in searchable map
		if (argc < 2) throw InteractionError("A controller program must be specified as the first argument.");
		String Controller = argv[1];

		for (unsigned int CurrentArgumentIndex = 2; CurrentArgumentIndex < (unsigned int)argc; CurrentArgumentIndex++)
		{
			String CurrentArgument = argv[CurrentArgumentIndex];
			if ((CurrentArgument == "--help") || (CurrentArgument == "-h"))
			{
				HelpMode = true;
				std::cout << "Command line arguments:\n"
					"\t--help|-h Shows this help.  If a controller is specified, shows help for information required by the controller as well.\n"
					"\t--verbose|-v While information is gathered, various statistics and gathered data will be shown.\n"
					"\n";
			}
			else if ((CurrentArgument == "--verbose") || (CurrentArgument == "-v"))
			{
				Verbose = true;
			}
			else ProgramArguments.And(CurrentArgument);
		}

		// Prepare information management
		std::map<String, Information::Anchor *> InformationItems;
		Information::AnchorImplementation<Blank> BlankInformation;
		InformationItems[Blank::GetIdentifier()] = &BlankInformation;
		Information::AnchorImplementation<Flag> FlagInformation;
		InformationItems[Flag::GetIdentifier()] = &FlagInformation;
		Information::AnchorImplementation<Platform> PlatformInformation;
		InformationItems[Platform::GetIdentifier()] = &PlatformInformation;
		Information::AnchorImplementation<InstallBinDirectory> BinInstallInformation;
		InformationItems[InstallBinDirectory::GetIdentifier()] = &BinInstallInformation;
		Information::AnchorImplementation<InstallDataDirectory> DataInstallInformation;
		InformationItems[InstallDataDirectory::GetIdentifier()] = &DataInstallInformation;
		Information::AnchorImplementation<InstallGlobalConfigDirectory> ConfigInstallInformation;
		InformationItems[InstallGlobalConfigDirectory::GetIdentifier()] = &ConfigInstallInformation;

		// Spawn script subprocess
		ChildInStream RequestStream;
		ChildOutStream ResponseStream;
		if (Controller != "--")
			ForkController(Controller, RequestStream, ResponseStream);
		else
		{
			// Allow the user to directly interact with self discovery if they specify "--" as the controller
			RequestStream.Associate(0);
			ResponseStream.Associate(1);
		}

		// Send initial paramers/header message
		if (HelpMode) ResponseStream.Write("help");
		ResponseStream.Write();

		// Read and process requests
		String FullLine;
		String RawLine;
		unsigned int RequestIndex = 0;
		while (true)
		{
			RawLine = RequestStream.ReadLine();
			if (RequestStream.HasFailed())
				break;

			if (!RawLine.empty())
				FullLine += RawLine;
			else
			{
				++RequestIndex; // Requests start from 1

				std::queue<String> Line = ParseInput(FullLine);
				FullLine = "";

				String Identifier;
				if (!Line.empty())
				{
					Identifier = Line.front();
					Line.pop();
				}

				std::map<String, Information::Anchor *>::iterator Information = InformationItems.find(Identifier);
				if (Information == InformationItems.end())
					throw InteractionError("Error: Unknown request type \"" + Identifier + "\" for request #" + AsString(RequestIndex) + ".");

				if (HelpMode)
				{
					Information->second->DisplayUserHelp(std::cout);
					std::cout << std::endl;
				}
				else
				{
					StringStream Out;
					Information->second->Respond(std::move(Line), Out);
					ResponseStream.Write(Out.str());
					ResponseStream.Write();
				}
			}
		}
	}
	catch (InteractionError &Error)
	{
		std::cerr << "Self discovery failed with error: " << Error.Message << std::endl;
		return 1;
	}
	catch (ControllerError &Error)
	{
		std::cerr << "The configuration script behaved incomprehensibly.  Check that you have the latest version of self discovery, and, if you do and the problem persists, please report this to the package maintainer:\n\t" << Error.Message << std::endl;
		return 1;
	}

	// Done! :)
	return 0;
}

