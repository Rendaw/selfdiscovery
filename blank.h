#ifndef BLANK_H
#define BLANK_H

#include <queue>

#include "ren-general/string.h"

class Blank
{
	public:
		static String GetIdentifier(void);
		static void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
};

#endif // BLANK_H

