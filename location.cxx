#include "location.h"

#ifdef WINDOWS
#include <shlobj.h>
#endif

#include "ren-general/arrangement.h"

#include "shared.h"
#include "platform.h"

extern Information::AnchorImplementation<Platform> PlatformInformation;

static String GetProjectName(std::queue<String> &Arguments)
	{ return GetNextArgument(Arguments, "project name"); }

#ifdef _WIN32
static void DumpProjectInstallDirectory(std::ostream &Out, String const &Project)
{
	wchar_t PathResult[MAX_PATH];
	HRESULT Result = SHGetFolderPathW(nullptr, CSIDL_PROGRAM_FILES, nullptr, 0, PathResult);
	if (Result != S_OK)
		throw InteractionError("Couldn't locate executable installation directory!  Received error " + AsString(Result));
	Out << AsString(PathResult) << "\\" << Project << "\n";
}
#endif


String InstallExecutableDirectory::GetIdentifier(void) { return "install-executable-directory"; }
		
void InstallExecutableDirectory::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " PROJECT\n"
		"\tResult: LOCATION\n"
		"\tReturns LOCATION, which will be the most appropriate executable installation location for project PROJECT.\n"
		"\n";
}

void InstallExecutableDirectory::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	Out << "\tprefix=LOCATION\n"
		"\tOverrides the default executable installation directory to use LOCATION/bin.\n\n";

	Out << "\t" << GetIdentifier() << "=LOCATION\n"
		"\tOverrides the default executable installation directory to use LOCATION.  If the 64-bit executable location is not overridden, LOCATION is also used for the 64-bit executable installation directory.\n\n";
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
		Out << OverrideExecutable.second << "\n";
		return;
	}

	std::pair<bool, String> OverridePrefix = FindProgramArgument("prefix");
	
#ifdef _WIN32
	if (OverridePrefix.first)
	{
		Out << OverridePrefix.second << "/bin\n";
		return;
	}
	DumpProjectInstallDirectory(Out, ProjectName);
#else
	assert(PlatformInformation->GetFamily() == Platform::Families::Linux);
	String Prefix = "/usr";
	if (OverridePrefix.first) Prefix = OverridePrefix.second;
	Out << Prefix << "/bin\n";
#endif
}

String InstallLibraryDirectory::GetIdentifier(void) { return "install-library-directory"; }
		
void InstallLibraryDirectory::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " PROJECT\n"
		"\tResult: LOCATION\n"
		"\tReturns LOCATION, which will be the most appropriate shared library installation location for project PROJECT.\n"
		"\n";
}

void InstallLibraryDirectory::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	Out << "\t" << "prefix=LOCATION\n"
		"\tOverrides the default library installation directory to use LOCATION/bin.\n\n";

	Out << "\t" << GetIdentifier() << "=LOCATION\n"
		"\tOverrides the default library installation directory to use LOCATION.  If the 64-bit library location is not overridden, LOCATION is also used for the 64-bit library installation directory.\n\n";
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
		Out << OverrideExecutable.second << "\n";
		return;
	}

	std::pair<bool, String> OverridePrefix = FindProgramArgument("prefix");
	
#ifdef _WIN32
	if (OverridePrefix.first)
	{
		Out << OverridePrefix.second << "/bin\n";
		return;
	}
	DumpProjectInstallDirectory(Out, ProjectName);
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
	Out << "\n";
#endif
}

String InstallDataDirectory::GetIdentifier(void) { return "install-data-directory"; }
		
void InstallDataDirectory::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " PROJECT\n"
		"\tResult: LOCATION\n"
		"\tReturns LOCATION, which will be the most appropriate application data installation location for project PROJECT.\n"
		"\n";
}

void InstallDataDirectory::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProjectName = GetProjectName(Arguments);
	Out << "\t" << "prefix=LOCATION\n"
		"\tOverrides the default data installation directory to use LOCATION/share/" << ProjectName << ".\n\n";

	Out << "\t" << GetIdentifier() << "=LOCATION\n"
		"\tOverrides the default data installation directory to use LOCATION.\n\n";
}

void InstallDataDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProjectName = GetProjectName(Arguments);
	std::pair<bool, String> OverrideData = FindProgramArgument(GetIdentifier());
	if (OverrideData.first)
	{
		Out << OverrideData.second << "\n";
		return;
	}

	std::pair<bool, String> OverridePrefix = FindProgramArgument("prefix");

#ifdef _WIN32
	if (OverridePrefix.first)
	{
		Out << OverridePrefix.second << "/share/" << ProjectName << "\n";
		return;
	}
	DumpProjectInstallDirectory(Out, ProjectName);
#else
	String Prefix = "/usr";
	if (OverridePrefix.first) Prefix = OverridePrefix.second;
	Out << Prefix << "/share/" << ProjectName << "\n";
#endif
}

String InstallGlobalConfigDirectory::GetIdentifier(void) { return "install-config-directory"; }
		
void InstallGlobalConfigDirectory::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " PROJECT\n"
		"\tResult: LOCATION\n"
		"\tReturns LOCATION, which will be the most appropriate system-wide configuration file installation location for project PROJECT.\n"
		"\n";
}

void InstallGlobalConfigDirectory::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProjectName = GetProjectName(Arguments);
	Out << "\t" << "prefix=LOCATION\n"
		"\tOverrides the default config installation directory to use LOCATION/etc/" << ProjectName << ".\n\n";

	Out << "\t" << GetIdentifier() << "=LOCATION\n"
		"\tOverrides the default data installation directory to use LOCATION.\n\n";
}

void InstallGlobalConfigDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProjectName = GetProjectName(Arguments);
	std::pair<bool, String> OverrideConfig = FindProgramArgument(GetIdentifier());
	if (OverrideConfig.first)
	{
		Out << OverrideConfig.second << "\n";
		return;
	}

	std::pair<bool, String> OverridePrefix = FindProgramArgument("prefix");
	if (OverridePrefix.first)
	{
		Out << OverridePrefix.second << "/etc/" << ProjectName << "\n";
		return;
	}
#ifdef WINDOWS
	wchar_t PathResult[MAX_PATH];
	HRESULT Result = SHGetFolderPathW(nullptr, CSIDL_COMMON_APPDATA, nullptr, 0, PathResult);
	if (Result != S_OK)
		throw InteractionError("Couldn't find global config directory!  Received error " + AsString(Result));
	Out << AsString(PathResult) << "\n";
#else
	Out << "/usr/etc" << "\n";
#endif
}

