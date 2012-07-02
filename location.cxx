#include "location.h"

#include "ren-general/arrangement.h"

#include "shared.h"

static String GetProjectName(std::queue<String> &Arguments)
	{ return GetNextArgument(Arguments, "project name"); }

String InstallBinDirectory::GetIdentifier(void) { return "install-binary-directory"; }

void InstallBinDirectory::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	Out << "\t" << "prefix=LOCATION\n"
		"Overrides the default binary installation directory to use LOCATION/bin.\n\n";
	Out << "\t" << GetIdentifier() << "=LOCATION\n"
		"Overrides the default binary installation directory to use LOCATION.  If the 64-bit binary location is not overridden, LOCATION is also used for the 64-bit binary installation directory.\n\n";
	Out << "\t" << GetIdentifier() << "-x64=LOCATION\n"
		"Overrides the default 64-bit binary installation directory to use LOCATION.\n\n";
}

#ifdef _WIN32
static void DumpProjectInstallDirectory(Set<String> const &Flags, std::ostream &Out, String const &Project)
{
	PWSTR PathResult;
	HRESULT Result = SHGetKnownFolderPath(Flags.Contains("x64") ? FOLDERID_ProgramFilesX64 : FOLDERID_ProgramFiles, 0, nullptr, &PathResult);
	if (Result != S_OK)
		throw InteractionError("Couldn't binary installation directory!  Received error " + AsString(Result));
	Out << PathResult << "\\" << Project << "\n\n";
	CoTaskMemFree(PathResult);
}
#endif

void InstallBinDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProjectName = GetProjectName(Arguments);
	Set<String> Flags;
	while (!Arguments.empty())
	{
		Flags.And(Arguments.front());
		Arguments.pop();
	}

	bool Is64Bit = Flags.Contains("x64");

	if (Is64Bit)
	{
		std::pair<bool, String> OverrideBin64 = FindProgramArgument(GetIdentifier());
		if (OverrideBin64.first)
		{
			Out << OverrideBin64.second << "\n\n";
			return;
		}
	}

	std::pair<bool, String> OverrideBin = FindProgramArgument(GetIdentifier());
	if (OverrideBin.first)
	{
		Out << OverrideBin.second << "\n\n";
		return;
	}

	std::pair<bool, String> OverridePrefix = FindProgramArgument("prefix");
	if (OverridePrefix.first)
	{
		Out << OverridePrefix.second << "/bin\n\n";
		return;
	}
	
#ifdef _WIN32
	DumpProjectInstallDirectory(Flags, Out, ProjectName);
#else
	Out << "/usr/local/bin\n\n";
#endif
}

String InstallDataDirectory::GetIdentifier(void) { return "install-data-directory"; }

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
	if (OverridePrefix.first)
	{
		Out << OverridePrefix.second << "/share/" << ProjectName << "\n\n";
		return;
	}
#ifdef _WIN32
	DumpProjectInstallDirectory(Out, ProjectName);
#else
	Out << "/usr/local/share/" << ProjectName << "\n\n";
#endif
}

String InstallGlobalConfigDirectory::GetIdentifier(void) { return "install-config-directory"; }

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
	Out << "/etc" << "\n\n";
#endif
}

