#ifndef CLIBRARY_H
#define CLIBRARY_H

#include <queue>

#include "ren-general/string.h"
#include "ren-general/filesystem.h"

class CLibrary
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		static void DisplayUserHelp(Script &State, std::map<String, String> &HelpItems);
		CLibrary(void);
		void Respond(Script &State);
	private:
		std::vector<DirectoryPath> const TestLocations;
};

#endif // CLIBRARY_H

