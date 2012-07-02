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
			static String const LinuxDebian;
			static String const LinuxArch;

			// BSD members
			static String const OpenBSD;
			static String const FreeBSD;
			static String const NetBSD;
		};

		Platform(void);
		static String GetIdentifier(void);
		static void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
		String const &GetFamily(void) const;
		String const &GetMember(void) const;

	private:
		String Family, Member;
};

#endif // PLATFORM_H

