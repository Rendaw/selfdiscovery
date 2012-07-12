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
		static void DisplayControllerHelp(std::ostream &Out);
		static void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
};

