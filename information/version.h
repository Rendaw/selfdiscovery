#ifdef VERSION_H
#error Include each header at most once per translation unit
#endif
#define VERSION_H

#include <queue>

#include "ren-general/string.h"

class Version
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		static void DisplayUserHelp(Script &State, std::map<String, String> &HelpItems);
		void Respond(Script &State);
};

