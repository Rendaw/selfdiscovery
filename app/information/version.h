#ifdef VERSION_H
#error Include each header at most once per translation unit
#endif
#define VERSION_H

#include "../information.h"

class Version
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		void Respond(Script &State, HelpItemCollector *);
};

