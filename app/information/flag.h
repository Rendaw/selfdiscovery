#ifndef FLAG_H
#define FLAG_H

#include "../information.h"

class Flag
{
	public:
		static String GetIdentifier(void);
		static void DisplayControllerHelp(void);
		void Respond(Script &State, HelpItemCollector *HelpItems);
};

#endif // FLAG_H

