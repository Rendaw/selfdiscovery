#ifndef LOCATION_H
#define LOCATION_H

#include <queue>

#include "ren-general/string.h"

class InstallExecutableDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		static void DisplayUserHelp(Script &State, HelpItemCollector &HelpItems);
		void Respond(Script &State);
};

class InstallLibraryDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		static void DisplayUserHelp(Script &State, HelpItemCollector &HelpItems);
		void Respond(Script &State);
};

class InstallDataDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		static void DisplayUserHelp(Script &State, HelpItemCollector &HelpItems);
		void Respond(Script &State);
};

class InstallGlobalConfigDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(std::ostream &Out);
		static void DisplayUserHelp(Script &State, HelpItemCollector &HelpItems);
		void Respond(Script &State);
};

#endif // LOCATION_H

