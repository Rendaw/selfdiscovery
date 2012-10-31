#ifdef CXXCOMPILER_H
#error Include each header at most once per translation unit.
#endif
#define CXXCOMPILER_H

#include "../information.h"

class CXXCompiler
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		static void DisplayUserHelp(Script &State, HelpItemCollector &HelpItems);
		void Respond(Script &State);
};

