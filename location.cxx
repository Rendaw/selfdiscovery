#include "location.h"

#include "ren-general/arrangement.h"

#include "shared.h"

String InstallBinDirectory::GetIdentifier(void) { return "install-binary-directory"; }

void InstallBinDirectory::DisplayUserHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " PROJECTNAME (x64)\n"
		"\tResponse: DIRECTORY\n"
		"Returns the system-defined binary installation directory for the project PROJECTNAME.  The location may or not exist, but it is appropriate to create it and write binary files into it.  Specify \"x64\" if the binaries are 64-bit, to assist determination of the correct location.\n\n";
}

#ifdef _WIN32
static void DumpProjectInstallDirectory(std::ostream &Out, String const &Project)
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
	if (Arguments.empty())
		throw ControllerError("install-bin-directory requires a project name.");
	String ProjectName = Arguments.front();
	Arguments.pop();
	Set<String> Flags;
	while (!Arguments.empty())
	{
		Flags.And(Arguments.front());
		Arguments.pop();
	}
#ifdef _WIN32
	DumpProjectInstallDirectory(Out, ProjectName);
#else
	Out << "/usr/local/bin\n\n";
#endif
}

String InstallDataDirectory::GetIdentifier(void) { return "install-data-directory"; }

void InstallDataDirectory::DisplayUserHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " PROJECTNAME (x64)\n"
		"\tResponse: DIRECTORY\n"
		"Returns the system-defined data installation directory for the project PROJECTNAME.  The location may or not exist, but it is appropriate to create it and write data files into it.  Specify \"x64\" if the project binaries are 64-bit, to assist determination of the correct location.\n\n";
}

void InstallDataDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	if (Arguments.empty())
		throw ControllerError("install-data-directory requires a project name.");
	String ProjectName = Arguments.front();
	Arguments.pop();
	Set<String> Flags;
	while (!Arguments.empty())
	{
		Flags.And(Arguments.front());
		Arguments.pop();
	}
#ifdef _WIN32
	DumpProjectInstallDirectory(Out, ProjectName);
#else
	Out << "/usr/local/share/" << ProjectName << "\n\n";
#endif
}

String InstallGlobalConfigDirectory::GetIdentifier(void) { return "install-config-directory"; }

void InstallGlobalConfigDirectory::DisplayUserHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " PROJECTNAME\n"
		"\tResponse: DIRECTORY\n"
		"Returns the system-wide configuration file installation directory for the project PROJECTNAME.  The location may or not exist, but it is appropriate to create it and write configuration files into it.\n\n";
}

void InstallGlobalConfigDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
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
