#ifndef LOCATION_H
#define LOCATION_H

#include <queue>

#include "ren-general/string.h"

class InstallBinDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
};

class InstallDataDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
};

class InstallGlobalConfigDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
};

#endif // LOCATION_H

