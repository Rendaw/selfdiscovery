// TODO selfdiscovery-version information item, fails if versions aren't compatible
// TODO fortune zodiak item
// TODO personality item

#include <iostream>
#include <cstring>
#include <queue>

#include "ren-general/string.h"
#include "ren-general/arrangement.h"
#include "ren-general/lifetime.h"

#include "shared.h"

// Global information and information types - used in main loop and in individual info types and such
Set<String> ProgramArguments;
enum RunModes { Normal, Help, ControllerHelp } RunMode = Normal;
bool Verbose = false;

#include "blank.h"
#include "flag.h"
#include "platform.h"
#include "location.h"
#include "program.h"
#include "cxxcompiler.h"
#include "clibrary.h"
		
Information::AnchorImplementation<Platform> PlatformInformation;
Information::AnchorImplementation<Program> ProgramInformation;

int main(int argc, char **argv)
{
	try {
		// Process arguments, store in searchable map
		if (argc < 2) throw InteractionError("A controller program must be specified as the first argument.");
		String ControllerName = argv[1];

		for (unsigned int CurrentArgumentIndex = 2; CurrentArgumentIndex < (unsigned int)argc; CurrentArgumentIndex++)
			ProgramArguments.And(argv[CurrentArgumentIndex]);

		if (FindProgramArgument("help").first || FindProgramArgument("--help").first || FindProgramArgument("-h").first) RunMode = RunModes::Help;
		if (FindProgramArgument("controllerhelp").first) RunMode = RunModes::ControllerHelp;
		if (FindProgramArgument("verbose").first) Verbose = true;

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
		Information::AnchorImplementation<Program> ProgramInformation;
		InformationItems[Program::GetIdentifier()] = &ProgramInformation;
		Information::AnchorImplementation<CXXCompiler> CXXCompilerInformation;
		InformationItems[CXXCompiler::GetIdentifier()] = &CXXCompilerInformation;
		Information::AnchorImplementation<CLibrary> CLibraryInformation;
		InformationItems[CLibrary::GetIdentifier()] = &CLibraryInformation;

		// Display controller help and exit early if that flag was set
		if (RunMode == RunModes::ControllerHelp)
		{
			std::cout << "\tA controller is a program that passes requests for information to this program and processes the returned information.  The controller is run as a subprocess of this program, and may be any generally recognized program type, such as an executable or shell script.  The controller is specified as the first argument of this program.\n"
				"\tEach request may be one or more lines of space-separated values, followed by a blank line.  The responses are one or more lines of information followed by a blank line.  A properly behaved controller should only exit before making a request, after this program's response has completed.\n"
				"\tAlternatively, you can specify \"-\" instead of a controller program to enter an interactive mode where you can type requests directly to the program.\n"
				"\n"
				"\tThe following requests are supported:\n\n";
			for (auto &InformationPair : InformationItems)
				InformationPair.second->DisplayControllerHelp(std::cout);
			return 0;
		}

		if (RunMode == RunModes::Help)
		{
			std::cout << "\tThis program communicates system information and user preferences to a controller program.  One use for this behavior is package configuration, where the controller takes the system information and prepares the package for compilation.  Perhaps that is why you are seeking help.\n"
				"\tThis program takes one argument, the controller.  Generally, this should be enough for the controller to complete its duties.  However, if the program fails to find necessary information or determines the wrong values, you may have to provide additional arguments.\n"
				"\tThe following flags can be passed as arguments to this program to override gathered information or control how it is gathered.  The flags below are listed based on the pieces of information the program requires, so they should all be immediately relevant.  No -- or - is required when specifying flags on the command line.  Run this again with the \"controllerhelp\" flag for documentation on creating a controller.  Run this with the \"verbose\" flag for more information on the information gathered as it is gathered.\n"
				"\n";
		}

		// Spawn script subprocess
		std::unique_ptr<Subprocess> Controller;
		SubprocessInStream WrappedStandardIn;
		SubprocessOutStream WrappedStandardOut;
		if (ControllerName != "--")
			Controller = std::unique_ptr<Subprocess>(new Subprocess(FilePath::Qualify(ControllerName), {}));
		else
		{
			// Allow the user to directly interact with self discovery if they specify "--" as the controller
			WrappedStandardIn.Associate(0);
			WrappedStandardOut.Associate(1);
		}
		SubprocessInStream &RequestStream = Controller ? Controller->In : WrappedStandardIn;
		SubprocessOutStream &ResponseStream = Controller ? Controller->Out : WrappedStandardOut;

		// Send initial paramers/header message
		if ((RunMode == RunModes::Help) || (RunMode == RunModes::ControllerHelp)) 
			ResponseStream.Write("help\n");
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

				try {
					assert(RunMode != RunModes::ControllerHelp);
					if (RunMode == RunModes::Help)
					{
						Information->second->DisplayUserHelp(std::move(Line), std::cout);
					}
					else
					{
						StringStream Out;
						Information->second->Respond(std::move(Line), Out);
						ResponseStream.Write(Out.str());
					}
					ResponseStream.Write();
				}
				catch (ControllerError &Error)
				{
					throw ControllerError("In request \"" + RawLine + "\": " + Error.Message);
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

