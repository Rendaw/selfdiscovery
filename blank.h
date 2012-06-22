#ifndef BLANK_H
#define BLANK_H

class Blank
{
	public:
		static String GetIdentifier(void);
		static void DisplayUserHelp(std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
};

#endif // BLANK_H

