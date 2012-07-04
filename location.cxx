#include "location.h"

#include "ren-general/arrangement.h"

#include "shared.h"
#include "platform.h"

extern Information::AnchorImplementation<Platform> PlatformInformation;

static String GetProjectName(std::queue<String> &Arguments)
	{ return GetNextArgument(Arguments, "project name"); }

#ifdef _WIN32
static void DumpProjectInstallDirectory(Set<String> const &Flags, std::ostream &Out, String const &Project)
{
	PWSTR PathResult;
	HRESULT Result = SHGetKnownFolderPath((PlatformInformation->GetArchitectureBits() == 64) ? FOLDERID_ProgramFilesX64 : FOLDERID_ProgramFiles, 0, nullptr, &PathResult);
	if (Result != S_OK)
		throw InteractionError("Couldn't executable installation directory!  Received error " + AsString(Result));
	Out << PathResult << "\\" << Project << "\n\n";
	CoTaskMemFree(PathResult);
}
#endif


String InstallExecutableDirectory::GetIdentifier(void) { return "install-executable-directory"; }
		
void InstallExecutableDirectory::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " PROJECT\n"
		"\tResult: LOCATION\n"
		"Returns LOCATION, which will be the most appropriate executable installation location for project PROJECT.\n"
		"\n";
}

void InstallExecutableDirectory::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	Out << "\tprefix=LOCATION\n"
		"Overrides the default executable installation directory to use LOCATION/bin.\n\n";
	Out << "\t" << GetIdentifier() << "=LOCATION\n"
		"Overrides the default executable installation directory to use LOCATION.  If the 64-bit executable location is not overridden, LOCATION is also used for the 64-bit executable installation directory.\n\n";
}

void InstallExecutableDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProjectName = GetProjectName(Arguments);
	Set<String> Flags;
	while (!Arguments.empty())
	{
		Flags.And(Arguments.front());
		Arguments.pop();
	}

	std::pair<bool, String> OverrideExecutable = FindProgramArgument(GetIdentifier());
	if (OverrideExecutable.first)
	{
		Out << OverrideExecutable.second << "\n\n";
		return;
	}

	std::pair<bool, String> OverridePrefix = FindProgramArgument("prefix");
	
#ifdef _WIN32
	if (OverridePrefix.first)
	{
		Out << OverridePrefix.second << "/bin\n\n";
		return;
	}
	DumpProjectInstallDirectory(Flags, Out, ProjectName);
#else
	assert(PlatformInformation->GetFamily() == Platform::Families::Linux);
	String Prefix = "/usr";
	if (OverridePrefix.first) Prefix = OverridePrefix.second;
	Out << Prefix << "/bin\n\n";
#endif
}

String InstallLibraryDirectory::GetIdentifier(void) { return "install-library-directory"; }
		
void InstallLibraryDirectory::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " PROJECT\n"
		"\tResult: LOCATION\n"
		"Returns LOCATION, which will be the most appropriate shared library installation location for project PROJECT.\n"
		"\n";
}

void InstallLibraryDirectory::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	Out << "\t" << "prefix=LOCATION\n"
		"Overrides the default library installation directory to use LOCATION/bin.\n\n";
	Out << "\t" << GetIdentifier() << "=LOCATION\n"
		"Overrides the default library installation directory to use LOCATION.  If the 64-bit library location is not overridden, LOCATION is also used for the 64-bit library installation directory.\n\n";
}

void InstallLibraryDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProjectName = GetProjectName(Arguments);
	Set<String> Flags;
	while (!Arguments.empty())
	{
		Flags.And(Arguments.front());
		Arguments.pop();
	}

	std::pair<bool, String> OverrideExecutable = FindProgramArgument(GetIdentifier());
	if (OverrideExecutable.first)
	{
		Out << OverrideExecutable.second << "\n\n";
		return;
	}

	std::pair<bool, String> OverridePrefix = FindProgramArgument("prefix");
	
#ifdef _WIN32
	if (OverridePrefix.first)
	{
		Out << OverridePrefix.second << "/bin\n\n";
		return;
	}
	DumpProjectInstallDirectory(Flags, Out, ProjectName);
#else
	assert(PlatformInformation->GetFamily() == Platform::Families::Linux);
	String Prefix = "/usr";
	if (OverridePrefix.first) Prefix = OverridePrefix.second;
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
	Out << "\n\n";
#endif
}

String InstallDataDirectory::GetIdentifier(void) { return "install-data-directory"; }
		
void InstallDataDirectory::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " PROJECT\n"
		"\tResult: LOCATION\n"
		"Returns LOCATION, which will be the most appropriate application data installation location for project PROJECT.\n"
		"\n";
}

void InstallDataDirectory::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProjectName = GetProjectName(Arguments);
	Out << "\t" << "prefix=LOCATION\n"
		"Overrides the default data installation directory to use LOCATION/share/" << ProjectName << ".\n\n";
	Out << "\t" << GetIdentifier() << "=LOCATION\n"
		"Overrides the default data installation directory to use LOCATION.\n\n";
}

void InstallDataDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProjectName = GetProjectName(Arguments);
	std::pair<bool, String> OverrideData = FindProgramArgument(GetIdentifier());
	if (OverrideData.first)
	{
		Out << OverrideData.second << "\n\n";
		return;
	}

	std::pair<bool, String> OverridePrefix = FindProgramArgument("prefix");

#ifdef _WIN32
	if (OverridePrefix.first)
	{
		Out << OverridePrefix.second << "/share/" << ProjectName << "\n\n";
		return;
	}
	DumpProjectInstallDirectory(Out, ProjectName);
#else
	String Prefix = "/usr";
	if (OverridePrefix.first) Prefix = OverridePrefix.second;
	Out << Prefix << "/share/" << ProjectName << "\n\n";
#endif
}

String InstallGlobalConfigDirectory::GetIdentifier(void) { return "install-config-directory"; }
		
void InstallGlobalConfigDirectory::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " PROJECT\n"
		"\tResult: LOCATION\n"
		"Returns LOCATION, which will be the most appropriate system-wide configuration file installation location for project PROJECT.\n"
		"\n";
}

void InstallGlobalConfigDirectory::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProjectName = GetProjectName(Arguments);
	Out << "\t" << "prefix=LOCATION\n"
		"Overrides the default config installation directory to use LOCATION/etc/" << ProjectName << ".\n\n";
	Out << "\t" << GetIdentifier() << "=LOCATION\n"
		"Overrides the default data installation directory to use LOCATION.\n\n";
}

void InstallGlobalConfigDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProjectName = GetProjectName(Arguments);
	std::pair<bool, String> OverrideConfig = FindProgramArgument(GetIdentifier());
	if (OverrideConfig.first)
	{
		Out << OverrideConfig.second << "\n\n";
		return;
	}

	std::pair<bool, String> OverridePrefix = FindProgramArgument("prefix");
	if (OverridePrefix.first)
	{
		Out << OverridePrefix.second << "/etc/" << ProjectName << "\n\n";
		return;
	}
#ifdef WINDOWS
	PWSTR PathResult;
	HRESULT Result = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, nullptr, &PathResult);
	if (Result != S_OK)
		throw InteractionError("Couldn't find global config directory!  Received error " + AsString(Result));
	Out << PathResult << "\n\n";
	CoTaskMemFree(PathResult);
#else
	Out << "/usr/etc" << "\n\n";
#endif
}
