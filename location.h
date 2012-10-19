#ifndef LOCATION_H
#define LOCATION_H

#include <queue>

#include "ren-general/string.h"
#include "ren-general/inputoutput.h"

class InstallExecutableDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(OutputStream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, OutputStream &Out);
		void Respond(std::queue<String> &&Arguments, OutputStream &Out);
};

class InstallLibraryDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(OutputStream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, OutputStream &Out);
		void Respond(std::queue<String> &&Arguments, OutputStream &Out);
};

class InstallDataDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(OutputStream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, OutputStream &Out);
		void Respond(std::queue<String> &&Arguments, OutputStream &Out);
};

class InstallGlobalConfigDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(OutputStream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, OutputStream &Out);
		void Respond(std::queue<String> &&Arguments, OutputStream &Out);
};

#endif // LOCATION_H

