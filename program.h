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
		static void DisplayControllerHelp(OutputStream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, OutputStream &Out);
		Program(void);
		void Respond(std::queue<String> &&Arguments, OutputStream &Out);
		FilePath *FindProgram(String const &ProgramName);
	private:
		std::vector<DirectoryPath> const Paths;
		DeleterMap<String, FilePath> Programs;
};

