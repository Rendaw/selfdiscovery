#ifdef PROGRAM_H
#error Include each header at most once per translation unit
#endif
#define PROGRAM_H

#include <queue>

#include "ren-general/string.h"
#include "ren-general/lifetime.h"
#include "ren-general/filesystem.h"

class Program
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		static void DisplayUserHelp(Script &State, std::map<String, String> &HelpItems);
		void Respond(Script &State);
		Program(void);
		FilePath *FindProgram(String const &ProgramName);
	private:
		std::vector<DirectoryPath> const Paths;
		DeleterMap<String, FilePath> Programs;
};
