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
enum RunModes { Normal, Help, ControllerHelp } RunMode = Normal;
bool Verbose = false;

#include "blank.h"
#include "version.h"
#include "flag.h"
#include "platform.h"
#include "location.h"
#include "program.h"
#include "cxxcompiler.h"
#include "clibrary.h"
		
Information::AnchorImplementation<Blank> BlankInformation;
Information::AnchorImplementation<Version> VersionInformation;
Information::AnchorImplementation<Flag> FlagInformation;
Information::AnchorImplementation<Platform> PlatformInformation;
Information::AnchorImplementation<InstallExecutableDirectory> ExecutableInstallInformation;
Information::AnchorImplementation<InstallLibraryDirectory> LibraryInstallInformation;
Information::AnchorImplementation<InstallDataDirectory> DataInstallInformation;
Information::AnchorImplementation<InstallGlobalConfigDirectory> ConfigInstallInformation;
Information::AnchorImplementation<Program> ProgramInformation;
Information::AnchorImplementation<CXXCompiler> CXXCompilerInformation;
Information::AnchorImplementation<CLibrary> CLibraryInformation;

int main(int argc, char **argv)
{
	try 
	{
		// Determine the controller
		if (argc < 2) throw InteractionError("A controller program must be specified as the first argument.");
		String ControllerName = argv[1];

		// Get configuration/overrides, prioritize closer stuff (commandline, user files) over more distant stuff (global config files)
		LoadConfiguragionFile(LocateGlobalConfigFile("selfdiscovery.config"));
		LoadConfiguragionFile(LocateUserConfigFile("selfdiscovery.config"));
		LoadConfiguragionFile(LocateWorkingDirectory().Select("selfdiscovery.config"));
		
		for (unsigned int CurrentArgumentIndex = 2; CurrentArgumentIndex < (unsigned int)argc; CurrentArgumentIndex++)
			LoadConfigurationCommandline(argv[CurrentArgumentIndex]);
		
		if (FindConfiguration("help").first || FindConfiguration("--help").first || FindConfiguration("-h").first) RunMode = RunModes::Help;
		if (FindConfiguration("controllerhelp").first) RunMode = RunModes::ControllerHelp;
		if (FindConfiguration("verbose").first) Verbose = true;

		if (Verbose)
		{
			for (auto ConfigurationElement : ProgramConfiguration)
				StandardStream << "Configuration setting: \'" << ConfigurationElement.first << "\' = \'" << ConfigurationElement.second.Value << "\', from " << ConfigurationElement.second.Source << "\n" << OutputStream::Flush();

		}

		// Prepare a list of information items for next operations
		DeleterList<Information::Anchor *> InformationItems = 
		{
			&BlankInformation,
			&VersionInformation,
			&FlagInformation,
			&PlatformInformation,
			&ExecutableInstallInformation,
			&LibraryInstallInformation,
			&DataInstallInformation,
			&ConfigInstallInformation,
			&ProgramInformation,
			&CXXCompilerInformation,
			&CLibraryInformation
		};

		// Display controller help and exit early if that flag was set
		if (RunMode == RunModes::ControllerHelp)
		{
			StandardStream << "\tA controller is a lua program that makes requests for information and processes the returned information.  The controller filename must be specified as the first argument to the program.  If you specify \"--\" instead of a filename, an interactive lua shell will be opened instead.\n"
				"\tThe following information queries can be made by the controller:\n\n";
			for (auto &InformationItem : InformationItems)
				InformationItem->DisplayControllerHelp();
			return 0;
		}

		if (RunMode == RunModes::Help)
		{
			StandardStream << "\tThis program gathers information about your system for a controller script.  Generally, this is used by software build scripts to configure themselves for your system.  This program takes one argument, the controller.  The controller controls which information should be gathered.\n"
				"\tIf this program cannot find required information or finds the incorrect information, you can override it by specifying an override on the command line or by putting the flag in a configuration file in a standard location.\n"
				"\tThe override file must be one of " << LocateGlobalConfigFile("selfdiscovery.config") << ", " << LocateUserConfigFile("selfdiscovery.config") << ", or " << LocateWorkingDirectory().Select("selfdiscovery.config") << ", ordered by increasing priority.  In the configuration files, only one override may be placed on a line.\n"
				"\tThe overrides should be specified in the form of \"overridename\" or \"overridename=value\".  Quotes and backslashes can be used to escape syntactic characters and group spaced words.  The following overrides are supported:\n"
				"\n";
		}

		// Prepare and run the controller
		Script ControlScript;
		ControlScript.CreateTable();

		HelpItemCollection HelpItems;
			
		for (auto &InformationItem : InformationItems)
		{
			if (RunMode == RunModes::Help)
				ControlScript.PushFunction(InformationItem->GetUserHelpCallback(HelpItems))
			else ControlScript.PushFunction(InformationItem->GetCallback())
			ControlScript.PutElement(InformationItem->GetName());
		}

		ControlScript.SaveGlobal("SelfDiscovery");

		if (ControllerName != "--")
			ControlScript.Do(ControllerName, Verbose);
		else
		{
			assert(0); // TODO
		}

		if (RunMode == RunModes::Help)
		{
			for (auto &HelpItem : HelpItems) 
			{
				StandardStream << "\t" << HelpItem.first << "\n";
				for (auto &Description : HelpItem.second)
			       		StandardStream << Description << "\n";
				StandardStream << "\n";
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

