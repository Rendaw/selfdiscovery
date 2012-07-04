#include "flag.h"

#include "ren-general/arrangement.h"

#include "shared.h"

String Flag::GetIdentifier(void) { return "flag"; }

static String GetFlagName(std::queue<String> &Arguments)
	{ return GetNextArgument(Arguments, "flag name"); }

void Flag::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " NAME DESCRIPTION\n"
		"\tResult: PRESENT VALUE\n"
		"Describes a flag a user may specify on the configuration command line.  If the user specifies the flag, PRESET is \"true\", otherwise \"false\".  If the user specifies the flag in the form NAME=VALUE, VALUE will be returned after PRESENT.\n"
		"\n";
}

void Flag::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	String FlagName = GetFlagName(Arguments);
	String FlagDescription = GetNextArgument(Arguments, "flag description");
	Out << "\t" << FlagName << "(=VALUE)\n"
		"Enables flag " << FlagName << " (and sets the value to VALUE).  " << FlagDescription << "\n\n";
}

void Flag::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	String FlagName = GetFlagName(Arguments);
	std::pair<bool, String> Flag = FindProgramArgument(FlagName);
	if (Flag.first)
		Out << "true " << Flag.second << "\n\n";
	else Out << "false" << "\n\n";
}

