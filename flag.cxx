#include "flag.h"

extern Set<String> ProgramArguments;

String Flag::GetIdentifier(void) { return "flag"; }

void Flag::DisplayUserHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " NAME\n"
		"\tResponse: true|false VALUE\n"
		"Looks for NAME in the command line argument list.  If NAME is present, responds with \"true\", otherwise responds with \"false\".  If the argument is specified in the form NAME=VALUE, the VALUE will be appended to the result.\n\n";
}

void Flag::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	if (Arguments.empty())
		throw ControllerError("The \"flag\" request requires a flag name.");
	String FlagName = Arguments.front();
	Arguments.pop();
	bool Found = false;
	Set<String>::iterator FoundFlag = ProgramArguments.find(FlagName);
	String Value;
	if (FoundFlag == ProgramArguments.end())
	{
		FoundFlag = ProgramArguments.lower_bound(FlagName);
		size_t EqualFound = FoundFlag->find('=');
		String Front;
		if (EqualFound != String::npos)
			Front = FoundFlag->substr(0, EqualFound);
		if (Front == FlagName)
		{
			Found = true;
			Value = FoundFlag->substr(EqualFound + 1, String::npos);
		}
	}
	else Found = true;

	if (Found == true)
		Out << "true " << value << "\n\n";
	else Out << "false" << "\n\n";
}

