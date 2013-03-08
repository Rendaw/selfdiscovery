#ifndef CLIBRARY_H
#define CLIBRARY_H

#include "../information.h"
#include "../ren-general/filesystem.h"

class CLibrary
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		CLibrary(void);
		void Respond(Script &State, HelpItemCollector *HelpItems);
	private:
		std::vector<DirectoryPath> const TestLocations;
};

#endif // CLIBRARY_H

