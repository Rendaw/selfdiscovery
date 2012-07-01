#include "platform.h"

#include <cassert>

extern bool Verbose;

String const Platform::Families::Windows = "windows";
String const Platform::Families::Linux = "linux";
String const Platform::Families::BSD = "bsd";
String const Platform::Members::Unknown = "unknown";
String const Platform::Members::Windows2000 = "win2000";
String const Platform::Members::WindowsXP = "winxp";
String const Platform::Members::WindowsServer2003 = "winserver2003";
String const Platform::Members::WindowsVista = "winvista";
String const Platform::Members::WindowsServer2008 = "winserver2008";
String const Platform::Members::Windows7 = "win7";
String const Platform::Members::Windows8 = "win8";
String const Platform::Members::WindowsServer2012 = "winserver2012";
String const Platform::Members::LinuxDebian = "debian";
String const Platform::Members::LinuxArch = "arch";
String const Platform::Members::OpenBSD = "openbsd";
String const Platform::Members::FreeBSD = "freebsd";
String const Platform::Members::NetBSD = "netbsd";

Platform::Platform(void) : Family(Families::Linux), Member(Members::Unknown)
{
#ifdef _WIN32
	Family = Families::Windows;
	OSVERSIONINFOEX VersionInfo;
	memset(&VersionInfo, 0, sizeof(VersionInfo));
	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
	GetVersionEx(&VersionInfo);
	if (VersionInfo.dwMajorVersion == 5)
	{
		if (VersionInfo.dwMinorVersion == 0)
			Member = Members::Windows2000;
		else if ((VersionInfo.dwMinorVersion == 1) || ((VersionInfo.dwMinorVersion == 2) && (VersionInfo.dwProductType == VER_NT_WORKSTATION)))
			Member = Members::WindowsXP;
		else Member = Members::WindowsServer2003;
	}
	else if (VersionInfo.dwMajorVersion == 6)
	{
		if (VersionInfo.dwProductType == VER_NT_WORKSTATION)
		{
			if (VersionInfo.dwMinorVersion == 0)
				Member = Members::WindowsVista;
			else if (VersionInfo.dwMinorVersion == 1)
				Member = Members::Windows7;
			else if (VersionInfo.dwMinorVersion == 2)
				Member = Members::Windows8;
		}
		else
		{
			if ((VersionInfo.dwMinorVersion == 0) || (VersionInfo.dwMinorVersion == 1))
				Member = Members::WindowsServer2008;
			else if (VersionInfo.dwMinorVersion == 2)
				Member = Members::WindowsServer2012;
		}
	}
#elif __OpenBSD__
	Family = Families::BSD;
	Member = Members::OpenBSD;
#elif __FreeBSD__
	Family = Families::BSD;
	Member = Members::FreeBSD;
#elif __NetBSD__
	Family = Families::BSD;
	Member = Members::NetBSD;
#elif sun
#elif __APPLE__
#else
	// Linux
	if (FilePath("/etc/debian_version").Exists())
		Member = Members::LinuxDebian;
	if (FilePath("/etc/arch-release").Exists())
		Member = Members::LinuxArch;
#endif
	if (Verbose)
		std::cout << "Detected platform: " << Family << ", " << Member << std::endl;
}

String Platform::GetIdentifier(void) { return "platform"; }

void Platform::DisplayUserHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << "\n"
		"\tResponse: FAMILY MEMBER\n"
		"Identifies and returns specifics about the operating system.  Possible FAMILY and MEMBER values follow in the format \"family: member1, member2...\".  Note, if the family is not recognized, it is assumed to be linux.  If the member is not recognized, it will be listed as \"unknown\".\n"
		"\t" << Families::Windows << ": " << Members::Windows2000 << ", " << Members::WindowsXP << ", " << Members::WindowsServer2003 << ", " << Members::WindowsVista << ", " << Members::WindowsServer2008 << ", " << Members::Windows7 << ", " << Members::Windows8 << ", " << Members::WindowsServer2012 << "\n"
		"\t" << Families::Linux << ": " << Members::LinuxDebian << ", " << Members::LinuxArch << "\n"
		"\t" << Families::BSD << ": " << Members::OpenBSD << ", " << Members::FreeBSD << ", " << Members::NetBSD << "\n"
		"\n";
}

void Platform::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	assert(!Family.empty());
	Out << Family << " " << Member << "\n\n";
}

String const &Platform::GetFamily(void) const { return Family; }

String const &Platform::GetMember(void) const { return Member; }

