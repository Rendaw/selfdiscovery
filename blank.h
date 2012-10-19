#ifndef BLANK_H
#define BLANK_H

#include <queue>

#include "ren-general/string.h"
#include "ren-general/inputoutput.h"

class Blank
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(OutputStream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, OutputStream &Out);
		void Respond(std::queue<String> &&Arguments, OutputStream &Out);
};

#endif // BLANK_H

