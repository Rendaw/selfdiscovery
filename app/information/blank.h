#ifndef BLANK_H
#define BLANK_H

#include <queue>

#include "ren-general/string.h"

class Blank
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		static void DisplayUserHelp(std::map<String, String> &HelpItems);
		void Respond(Script &State);
};

#endif // BLANK_H

