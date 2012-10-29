#ifndef FLAG_H
#define FLAG_H

#include <queue>

#include "ren-general/string.h"

class Flag
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		static void DisplayUserHelp(Script &State, std::map<String, String> &HelpItems);
		void Respond(Script &State);
};

#endif // FLAG_H

