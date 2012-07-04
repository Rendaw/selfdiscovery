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
		
Information::AnchorImplementation<Platform> PlatformInformation;

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
			char const NewLine[] = "\n";
			int Wrote = write(FileDescriptor, NewLine, sizeof(NewLine) - 1);
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
		OutStream.Associate(ToChild[WriteEnd]);

		close(FromChild[WriteEnd]);
		InStream.Associate(FromChild[ReadEnd]);
	}
#endif
}

int main(int argc, char **argv)
{
#ifdef NDEBUG
	try {
#endif // NDEBUG
		// Process arguments, store in searchable map
		if (argc < 2) throw InteractionError("A controller program must be specified as the first argument.");
		String Controller = argv[1];

		for (unsigned int CurrentArgumentIndex = 2; CurrentArgumentIndex < (unsigned int)argc; CurrentArgumentIndex++)
			ProgramArguments.And(argv[CurrentArgumentIndex]);

		if (FindProgramArgument("help").first) HelpMode = true;
		if (FindProgramArgument("verbose").first) Verbose = true;
		assert(!HelpMode); // DEBUG
		assert(Verbose); // DEBUG

		// Prepare information management
		std::map<String, Information::Anchor *> InformationItems;
		Information::AnchorImplementation<Blank> BlankInformation;
		InformationItems[Blank::GetIdentifier()] = &BlankInformation;
		Information::AnchorImplementation<Flag> FlagInformation;
		InformationItems[Flag::GetIdentifier()] = &FlagInformation;
		InformationItems[Platform::GetIdentifier()] = &PlatformInformation;
		Information::AnchorImplementation<InstallExecutableDirectory> ExecutableInstallInformation;
		InformationItems[InstallExecutableDirectory::GetIdentifier()] = &ExecutableInstallInformation;
		Information::AnchorImplementation<InstallLibraryDirectory> LibraryInstallInformation;
		InformationItems[InstallLibraryDirectory::GetIdentifier()] = &LibraryInstallInformation;
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

				std::queue<String> Line = SplitString(FullLine, {' ', '\t', '\n', '\r'}, true);
				if (Verbose) std::cout << "Processing request: " << FullLine << std::endl;
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

#ifdef NDEBUG
				try {
#endif // NDEBUG
					if (HelpMode)
					{
						Information->second->DisplayUserHelp(std::move(Line), std::cout);
						std::cout << std::endl;
					}
					else
					{
						StringStream Out;
						Information->second->Respond(std::move(Line), Out);
						ResponseStream.Write(Out.str());
						ResponseStream.Write();
					}
#ifdef NDEBUG
				}
				catch (ControllerError &Error)
				{
					throw ControllerError("In request \"" + RawLine + "\": " + Error.Message);
				}
#endif // NDEBUG
			}
		}
#ifdef NDEBUG
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
#endif // NDEBUG

	// Done! :)
	return 0;
}

