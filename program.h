static_assert(PROGRAM_H == 0, "Include each header at most once per translation unit.");
#define PROGRAM_H 1

#include <queue>

#include "ren-general/string.h"

class Program
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(std::ostream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out);
		Program(void);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
		DirectoryPath *FindProgram(String const &ProgramName);
	private:
		std::vector<DirectoryPath> const Paths;
		std::map<String, FilePath> Programs;
};

