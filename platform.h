#ifndef PLATFORM_H
#define PLATFORM_H

#include <queue>
#include <iostream>

#include "ren-general/string.h"
#include "ren-general/filesystem.h"

class Platform
{
	public:
		struct Families
		{
			static String const Windows;
			static String const Linux;
			static String const BSD;
		};
		struct Members
		{
			static String const Unknown;

			// Windows members
			static String const Windows2000;
			static String const WindowsXP;
			static String const WindowsServer2003;
			static String const WindowsVista;
			static String const WindowsServer2008;
			static String const Windows7;
			static String const Windows8;
			static String const WindowsServer2012;

			// Linux members
			static String const LinuxRedHat;
			static String const LinuxDebian;
			static String const LinuxUbuntu;
			static String const LinuxArch;

			// BSD members
			static String const OpenBSD;
			static String const FreeBSD;
			static String const NetBSD;
		};
		struct LinuxClasses
		{
			static String const Invalid;
			static String const RedHat;
			static String const Debian;
			static String const Slackware;
			static String const Arch;
		};

		Platform(void);
		static String GetIdentifier(void);
		static void DisplayControllerHelp(std::ostream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
		String const &GetFamily(void) const;
		String const &GetMember(void) const;
		unsigned int GetArchitectureBits(void) const;
		String const &GetLinuxClass(void) const;

	private:
		String Family, Member;
		unsigned int ArchitectureBits;
		String LinuxClass;
};

#endif // PLATFORM_H

