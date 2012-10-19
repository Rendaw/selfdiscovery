#ifdef VERSION_H
#error Include each header at most once per translation unit
#endif
#define VERSION_H

#include <queue>

#include "ren-general/string.h"
#include "ren-general/inputoutput.h"

class Version
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(OutputStream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, OutputStream &Out);
		void Respond(std::queue<String> &&Arguments, OutputStream &Out);
};

