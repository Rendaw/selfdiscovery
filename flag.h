#ifndef FLAG_H
#define FLAG_H

#include <queue>

#include "ren-general/string.h"
#include "ren-general/inputoutput.h"

class Flag
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(OutputStream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, OutputStream &Out);
		void Respond(std::queue<String> &&Arguments, OutputStream &Out);
};

#endif // FLAG_H

