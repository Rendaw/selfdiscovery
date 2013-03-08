// TODO fortune zodiak item
// TODO personality item

#include <iostream>
#include <cstring>
#include <queue>

#include "ren-general/string.h"
#include "ren-general/arrangement.h"
#include "ren-general/lifetime.h"

#include "shared.h"
#include "configuration.h"
#include "shellutility.h"

// Global information and information types - used in main loop and in individual info types and such
enum RunModes { Normal, Help, ControllerHelp } RunMode = Normal;
bool Verbose = false;

#include "information/version.h"
#include "information/flag.h"
#include "information/platform.h"
#include "information/location.h"
#include "information/program.h"
#include "information/cxxcompiler.h"
#include "information/clibrary.h"
		
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

std::vector<const char *> HelpNames = {"Help", "--help", "-h", "ControllerHelp"};

std::vector<FilePath> ConfigurationFilePaths = {
	LocateGlobalConfigFile("selfdiscovery.config"),
	LocateUserConfigFile("selfdiscovery.config"),
	LocateWorkingDirectory().Select("selfdiscovery.config")};

int main(int argc, char **argv)
{
	try 
	{
		// Determine the controller
		String ControllerName;
		if (argc >= 2) 
			ControllerName = argv[1];

		for (auto &HelpName : HelpNames)
			if (ControllerName == HelpName)
			{
				LoadConfigurationCommandline(ControllerName);
				ControllerName = String();
				break;
			}

		// Get configuration/overrides, prioritize closer stuff (commandline, user files) over more distant stuff (global config files)
		for (auto &ConfigurationFilePath : ConfigurationFilePaths)
			LoadConfigurationFile(ConfigurationFilePath);
		
		for (unsigned int CurrentArgumentIndex = 2; CurrentArgumentIndex < (unsigned int)argc; CurrentArgumentIndex++)
			LoadConfigurationCommandline(argv[CurrentArgumentIndex]);
		
		if (FindConfiguration("Help").first || FindConfiguration("--help").first || FindConfiguration("-h").first) RunMode = RunModes::Help;
		if (FindConfiguration("ControllerHelp").first) RunMode = RunModes::ControllerHelp;
		if (FindConfiguration("Verbose").first) Verbose = true;
		if (ControllerName.empty() && (RunMode == RunModes::Normal)) RunMode = RunModes::Help;

		if (Verbose)
		{
			for (auto ConfigurationElement : GetProgramConfiguration())
				StandardStream << "Read configuration setting \'" << ConfigurationElement.first << "\' ( = \'" << ConfigurationElement.second.Value << "\') from " << ConfigurationElement.second.Source << ".\n" << OutputStream::Flush();

		}

		// Prepare a list of information items for next operations
		std::list<Information::Anchor *> InformationItems({
			&VersionInformation,
			&FlagInformation,
			&PlatformInformation,
			&ExecutableInstallInformation,
			&LibraryInstallInformation,
			&DataInstallInformation,
			&ConfigInstallInformation,
			&ProgramInformation,
			&CXXCompilerInformation,
			&CLibraryInformation});

		// Display controller help and exit early if that flag was set
		if (RunMode == RunModes::ControllerHelp)
		{
			StandardStream << "\tA controller is a lua program that makes requests for information and processes the returned information.  The controller filename must be specified as the first argument to the program.\n"
				"\tIf the controller is invoked in help mode, all information queries will return nil and the controller should refrain from changing the system state.  The controller can check for help mode using Discover.HelpMode(), which returns true if help mode is active and false otherwise.\n"
				"\tThe following shell-style utility methods are provided to ease scripting:\n\n";
			ShowShellUtilityHelp();
			StandardStream <<
				"\tThe following information queries can be made by the controller:\n\n";
			for (auto &InformationItem : InformationItems)
				InformationItem->DisplayControllerHelp();
			return 0;
		}

		if (RunMode == RunModes::Help)
		{
			StandardStream << 
				"\tselfdiscovery CONFIGURATION...\n"
				"\tselfdiscovery CONTROLLER CONFIGURATION...\n"
				"\n"
				"\tThis program gathers information about your system for a controller script.  Generally, this is used by software build scripts to configure themselves for your system.  The controller script filename is specified by CONTROLLER.  The controller tells this program which information it should gather.\n"
				"\tCONFIGURATION... can be any number of the following values in addition to the items in the next section: Help, ControllerHelp, Verbose.  Help displays this message.  ControllerHelp displays documentation for writing controller scripts.  Verbose displays messages while discovery is in progress that are intended to clarify how and what information is being found.  If you specify CONTROLLER as well as Help, additional flags that can be used to override or guide information discovery will be listed below.\n"
				"\tAny values you can specify in CONFIGURATION... can also be placed in configuration files that will be automatically loaded.  Only one value may be specified per line.  The values loaded from the configuration files will supplement the CONFIGURATION... specified in the command line, but have lower precedence than the command line values.  The configuration files automatically loaded are, by increasing precedence: \n";
			for (auto &ConfigurationFilePath : ConfigurationFilePaths)
				StandardStream << "\t" << ConfigurationFilePath << "\n";
			StandardStream <<
				"\n"
				"\tExamples:\n"
				"\tselfdiscovery Help\n"
				"\tselfdiscovery ControllerHelp\n"
				"\tselfdiscovery example.lua Help\n"
				"\tselfdiscovery example.lua\n"
				"\tselfdiscovery example.lua Verbose Path=\"/usr/local/bin\"\n"
				"\n";
			if (!ControllerName.empty())
				StandardStream << "\tAdditional CONFIGURATION... values relevant to this controller:\n\n";
		}

		// Prepare and run the controller
		Script ControlScript;

		ControlScript.PushTable();
		RegisterShellUtilities(ControlScript);
		ControlScript.SaveGlobal("Utility");

		ControlScript.PushTable();
		ControlScript.PushFunction([&RunMode](Script &State)
		{
			if (RunMode == RunModes::Help) State.PushBoolean(true);
			else State.PushBoolean(false);
			return 1;
		});
		ControlScript.PutElement("HelpMode");

		ControlScript.PushFunction([&ControllerName](Script &State)
		{
			State.PushString(FilePath::Qualify(ControllerName).Directory());
			return 1;
		});
		ControlScript.PutElement("ControllerLocation");

		HelpItemCollector HelpItems;
			
		for (auto &InformationItem : InformationItems)
		{
			ControlScript.PushFunction(InformationItem->GetCallback(
				RunMode == RunModes::Help ? &HelpItems : nullptr));
			ControlScript.PutElement(InformationItem->GetIdentifier());
		}

		ControlScript.SaveGlobal("Discover");

		if (!ControllerName.empty())
			ControlScript.Do(ControllerName, Verbose);

		if (RunMode == RunModes::Help)
		{
			for (auto &HelpItem : HelpItems) 
			{
				StandardStream << "\t" << HelpItem.first << "\n";
				for (auto &Description : HelpItem.second)
			       		StandardStream << "\t" << Description << "\n";
				StandardStream << "\n";
			}
		}
	}
	catch (InteractionError &Failure)
	{
		StandardErrorStream << "Self discovery failed with error: " << Failure.Explanation << "\n" << OutputStream::Flush();
		return 1;
	}
	catch (Error::Input &Failure)
	{
		StandardErrorStream << "The configuration script behaved incomprehensibly.  Check that you have the latest version of self discovery, and, if you do and the problem persists, please report this to the package maintainer:\n\t" << Failure.Explanation << "\n" << OutputStream::Flush();
		return 1;
	}

	// Done! :)
	return 0;
}

