#ifdef PROGRAM_H
#error Include each header at most once per translation unit
#endif
#define PROGRAM_H

#include "../information.h"
#include "../ren-general/lifetime.h"
#include "../ren-general/filesystem.h"

class Program
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		void Respond(Script &State, HelpItemCollector *HelpItems);
		Program(void);
		FilePath *FindProgram(String const &ProgramName);
	private:
		std::vector<DirectoryPath> const Paths;
		DeleterMap<String, FilePath> Programs;
};

