#include "platform.h"

Platform::Platform(void) : Family("linux"), Member("unknown") 
{
#ifdef _WIN32
	Family = "windows";
	OSVERSIONINFOEX VersionInfo;
	memset(&VersionInfo, 0, sizeof(VersionInfo));
	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
	GetVersionEx(&VersionInfo);
	if (VersionInfo.dwMajorVersion == 5)
	{
		if (VersionInfo.dwMinorVersion == 0)
			Member = "win2000";
		else if ((VersionInfo.dwMinorVersion == 1) || ((VersionInfo.dwMinorVersion == 2) && (VersionInfo.dwProductType == VER_NT_WORKSTATION)))
			Member = "winxp";
		else Member = "server2003";
	}
	else if (VersionInfo.dwMajorVersion == 6)
	{
		if (VersionInfo.dwProductType == VER_NT_WORKSTATION)
		{
			if (VersionInfo.dwMinorVersion == 0)
				Member = "winvista";
			else if (VersionInfo.dwMinorVersion == 1)
				Member = "win7";
			else if (VersionInfo.dwMinorVersion == 2)
				Member = "win8";
		}
		else
		{
			if ((VersionInfo.dwMinorVersion == 0) || (VersionInfo.dwMinorVersion == 1))
				Member = "server2008";
			else if (VersionInfo.dwMinorVersion == 2)
				Member = "server2012";
		}
	}
#elif __OpenBSD__
#elif __FreeBSD__
#elif __NetBSD__
#elif sun
#elif __APPLE__
#else
	// Linux
	FilePath DistroVersionFile("/etc/debian_version");
	if (DistroVersionFile.Exists())
		Member = "debian";
#endif
}

String Platform::GetIdentifier(void) { return "platform"; }

void Platform::DisplayUserHelp(std::ostream &Out) 
{
	Out << "\t" << GetIdentifier() << "\n"
		"\tResponse: FAMILY MEMBER\n"
		"Identifies and returns specifics about the operating system.  Possible FAMILY and MEMBER values follow in the format \"family: member1, member2...\".  Note, if the family is not recognized, it is assumed to be linux.  If the member is not recognized, it will be listed as \"unknown\".\n"
		"\twindows: win2000, winxp, server2003, winvista, server2008, win7, win8, server2012\n"
		"\tlinux: redhat, centos, debian, fedora, ubuntu, arch, suse, slackware\n"
		"\tmacosx: 10.5, 10.6, 10.7\n"
		"\tbsd: freebsd, openbsd, netbsd\n"
		"\tsolaris: 9, 10, 11express, 11\n\n";
}

void Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	assert(!Family.empty());
	Out << Family << " " << Member << "\n\n";
}

String Platform::GetFamily(void) { return Family; }

String Platform::GetMember(void) { return Member; }

