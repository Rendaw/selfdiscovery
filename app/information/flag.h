#ifndef FLAG_H
#define FLAG_H

#include "../information.h"

class Flag
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		static void DisplayUserHelp(Script &State, HelpItemCollector &HelpItems);
		void Respond(Script &State);
};

#endif // FLAG_H

