static_assert(CXXCOMPILER_H == 0, "Include each header at most once per translation unit.");
#define CXXCOMPILER_H 1

#include <queue>

#include "ren-general/string.h"

class CXXCompiler
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(std::ostream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
};

