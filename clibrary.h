#ifndef CLIBRARY_H
#define CLIBRARY_H

#include <queue>

#include "ren-general/string.h"
#include "ren-general/filesystem.h"

class CLibrary
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(OutputStream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, OutputStream &Out);
		CLibrary(void);
		void Respond(std::queue<String> &&Arguments, OutputStream &Out);
	private:
		std::vector<DirectoryPath> const TestLocations;
};

#endif // CLIBRARY_H

