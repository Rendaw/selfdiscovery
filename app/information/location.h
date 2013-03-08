#ifndef LOCATION_H
#define LOCATION_H

#include "../information.h"

class InstallExecutableDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		void Respond(Script &State, HelpItemCollector *HelpItems);
};

class InstallLibraryDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		void Respond(Script &State, HelpItemCollector *HelpItems);
};

class InstallDataDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		void Respond(Script &State, HelpItemCollector *HelpItems);
};

class InstallGlobalConfigDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		void Respond(Script &State, HelpItemCollector *HelpItems);
};

#endif // LOCATION_H

