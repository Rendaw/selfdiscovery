#include "location.h"

#ifdef WINDOWS
#include <shlobj.h>
#endif

#include "../ren-general/arrangement.h"

#include "../shared.h"
#include "../configuration.h"
#include "platform.h"

extern bool Verbose;

extern Information::AnchorImplementation<Platform> PlatformInformation;

#ifdef _WIN32
static void DumpProjectInstallDirectory(Script &State, String const &Project)
{
	wchar_t PathResult[MAX_PATH];
	HRESULT Result = SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES, nullptr, 0, PathResult);
	if (Result != S_OK)
		throw InteractionError("Couldn't locate executable installation directory!  Received error " + AsString(Result));
	State.PushTable();
	State.PushString(AsString(NativeString(reinterpret_cast<char16_t *>(PathResult))) + "\\" + Project);
	State.PutElement("Location");
}
#endif

void ReturnLocation(Script &State, String const &Location)
{
	State.PushTable();
	State.PushString(Location);
	State.PutElement("Location");
}

String InstallExecutableDirectory::GetIdentifier(void) { return "InstallExecutableDirectory"; }
		
void InstallExecutableDirectory::DisplayControllerHelp(void)
{
	StandardStream << "\tDiscover." << GetIdentifier() << "{Project = PROJECT}\n"
		"\tResult: {Location = LOCATION}\n"
		"\tReturns LOCATION, which will be the most appropriate executable installation location for project PROJECT.\n"
		"\n";
}

void InstallExecutableDirectory::Respond(Script &State, HelpItemCollector *HelpItems)
{
	String ProjectName = GetArgument(State, "Project");
	ClearArguments(State);
	
	if (HelpItems != nullptr)
	{
		HelpItems->Add("Prefix=LOCATION", "Overrides the default executable installation directory to use LOCATION/bin.");
		HelpItems->Add(GetIdentifier() + "=LOCATION", "Overrides the default executable installation directory to use LOCATION.  If the 64-bit executable location is not overridden, LOCATION is also used for the 64-bit executable installation directory.");
	}

	std::pair<bool, String> OverrideExecutable = FindConfiguration(GetIdentifier());
	if (OverrideExecutable.first) 
	{
		if (Verbose) StandardStream << "Found " << GetIdentifier() << " for project \"" << ProjectName << "\" by configuration.\n" << OutputStream::Flush();
		ReturnLocation(State, OverrideExecutable.second);
		return;
	}

	std::pair<bool, String> OverridePrefix = FindConfiguration("Prefix");
	
#ifdef _WIN32
	if (OverridePrefix.first)
	{
		ReturnLocation(State, OverridePrefix.second + "/bin");
		return;
	}
	DumpProjectInstallDirectory(State, ProjectName);
#else
	assert(PlatformInformation->GetFamily() == Platform::Families::Linux);
	String Prefix = "/usr";
	if (OverridePrefix.first) Prefix = OverridePrefix.second;
	ReturnLocation(State, Prefix + "/bin");
#endif
}

String InstallLibraryDirectory::GetIdentifier(void) { return "InstallLibraryDirectory"; }
		
void InstallLibraryDirectory::DisplayControllerHelp(void)
{
	StandardStream << "\tSelfDiscovery." << GetIdentifier() << "{Project = PROJECT}\n"
		"\tResult: {Location = LOCATION}\n"
		"\tReturns LOCATION, which will be the most appropriate shared library installation location for project PROJECT.\n"
		"\n";
}

void InstallLibraryDirectory::Respond(Script &State, HelpItemCollector *HelpItems)
{
	String ProjectName = GetArgument(State, "Project");
	ClearArguments(State);

	if (HelpItems != nullptr)
	{
		HelpItems->Add("Prefix=LOCATION", "Overrides the default library installation directory to use LOCATION/bin.");
		HelpItems->Add(GetIdentifier() + "=LOCATION", "Overrides the default library installation directory to use LOCATION.  If the 64-bit library location is not overridden, LOCATION is also used for the 64-bit library installation directory.");
	}

	std::pair<bool, String> OverrideLibrary = FindConfiguration(GetIdentifier());
	if (OverrideLibrary.first)
	{
		if (Verbose) StandardStream << "Found " << GetIdentifier() << " for project \"" << ProjectName << "\" by configuration.\n" << OutputStream::Flush();
		ReturnLocation(State, OverrideLibrary.second);
		return;
	}

	std::pair<bool, String> OverridePrefix = FindConfiguration("Prefix");
	
#ifdef _WIN32
	if (OverridePrefix.first)
	{
		ReturnLocation(State, OverridePrefix.second + "/bin");
		return;
	}
	DumpProjectInstallDirectory(State, ProjectName);
#else
	assert(PlatformInformation->GetFamily() == Platform::Families::Linux);
	String Prefix = "/usr";
	if (OverridePrefix.first) Prefix = OverridePrefix.second;
	MemoryStream Out;
	Out << Prefix;
	if (PlatformInformation->GetLinuxClass() == Platform::LinuxClasses::Debian)
	{
		if (PlatformInformation->GetArchitectureBits() == 64) Out << "/lib";
		else Out << "/lib32";
	}
	else if (PlatformInformation->GetLinuxClass() == Platform::LinuxClasses::RedHat)
	{
		if (PlatformInformation->GetArchitectureBits() == 64) Out << "/lib64";
		else Out << "/lib";
	}
	ReturnLocation(State, Out);
#endif
}

String InstallDataDirectory::GetIdentifier(void) { return "InstallDataDirectory"; }
		
void InstallDataDirectory::DisplayControllerHelp(void)
{
	StandardStream << "\tDiscover." << GetIdentifier() << "{Project = PROJECT}\n"
		"\tResult: {Location = LOCATION}\n"
		"\tReturns LOCATION, which will be the most appropriate application data installation location for project PROJECT.\n"
		"\n";
}

void InstallDataDirectory::Respond(Script &State, HelpItemCollector *HelpItems)
{
	String ProjectName = GetArgument(State, "Project");
	ClearArguments(State);

	if (HelpItems != nullptr)
	{
		HelpItems->Add("Prefix=LOCATION", "Overrides the default data installation directory to use LOCATION/share/" + ProjectName + ".");
		HelpItems->Add(GetIdentifier() + "=LOCATION", "Overrides the default data installation directory to use LOCATION.");
	}

	std::pair<bool, String> OverrideData = FindConfiguration(GetIdentifier());
	if (OverrideData.first)
	{
		if (Verbose) StandardStream << "Found " << GetIdentifier() << " for project \"" << ProjectName << "\" by configuration.\n" << OutputStream::Flush();
		ReturnLocation(State, OverrideData.second);
		return;
	}

	std::pair<bool, String> OverridePrefix = FindConfiguration("Prefix");

#ifdef _WIN32
	if (OverridePrefix.first)
	{
		ReturnLocation(State, OverridePrefix.second + "/share/" + ProjectName);
		return;
	}
	DumpProjectInstallDirectory(State, ProjectName);
#else
	String Prefix = "/usr";
	if (OverridePrefix.first) Prefix = OverridePrefix.second;
	ReturnLocation(State, Prefix + "/share/" + ProjectName);
#endif
}

String InstallGlobalConfigDirectory::GetIdentifier(void) { return "InstallConfigDirectory"; }
		
void InstallGlobalConfigDirectory::DisplayControllerHelp(void)
{
	StandardStream << "\tDiscover." << GetIdentifier() << "{Project = PROJECT}\n"
		"\tResult: {Location = LOCATION}\n"
		"\tReturns LOCATION, which will be the most appropriate system-wide configuration file installation location for project PROJECT.\n"
		"\n";
}

void InstallGlobalConfigDirectory::Respond(Script &State, HelpItemCollector *HelpItems)
{
	String ProjectName = GetArgument(State, "Project");
	ClearArguments(State);

	if (HelpItems != nullptr)
	{
		HelpItems->Add("Prefix=LOCATION", "Overrides the default config installation directory to use LOCATION/etc/" + ProjectName + ".");
		HelpItems->Add(GetIdentifier() + "=LOCATION", "Overrides the default data installation directory to use LOCATION.");
	}

	std::pair<bool, String> OverrideConfig = FindConfiguration(GetIdentifier());
	if (OverrideConfig.first)
	{
		if (Verbose) StandardStream << "Found " << GetIdentifier() << " for project \"" << ProjectName << "\" by configuration.\n" << OutputStream::Flush();
		ReturnLocation(State, OverrideConfig.second);
		return;
	}

	std::pair<bool, String> OverridePrefix = FindConfiguration("prefix");
	if (OverridePrefix.first)
	{
		ReturnLocation(State, OverridePrefix.second + "/etc/" + ProjectName);
		return;
	}
#ifdef WINDOWS
	wchar_t PathResult[MAX_PATH];
	HRESULT Result = SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, 0, PathResult);
	if (Result != S_OK)
		throw InteractionError("Couldn't find global config directory!  Received error " + AsString(Result));
	ReturnLocation(State, AsString(NativeString(reinterpret_cast<char16_t *>(PathResult))));
#else
	ReturnLocation(State, "/usr/etc");
#endif
}

