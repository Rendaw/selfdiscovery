#ifndef CLIBRARY_H
#define CLIBRARY_H

#include "../information.h"
#include "../ren-general/filesystem.h"

class CLibrary
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		static void DisplayUserHelp(Script &State, HelpItemCollector &HelpItems);
		CLibrary(void);
		void Respond(Script &State);
	private:
		std::vector<DirectoryPath> const TestLocations;
};

#endif // CLIBRARY_H

