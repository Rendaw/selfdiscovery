#include "flag.h"

#include "ren-general/arrangement.h"

#include "shared.h"

String Flag::GetIdentifier(void) { return "flag"; }

static String GetFlagName(std::queue<String> &Arguments)
	{ return GetNextArgument(Arguments, "flag name"); }

void Flag::DisplayControllerHelp(void)
{
	StandardStream << "\tSelfDiscovery." << GetIdentifier() << "{Name = NAME(, Description = DESCRIPTION, HasValue = HASVALUE)}\n"
		"\tResult: {Present = PRESENT(, Value = VALUE)}\n"
		"\tDescribes a flag a user may specify on the configuration command line.  If the user specifies the flag, PRESENT is \"true\", otherwise \"false\".  If the user specifies the flag in the form NAME=VALUE, VALUE will also be returned.  If HASVALUE is specified as true, the user help will indicate that this flag should receive a value.\n";
}

void Flag::DisplayUserHelp(Script &State, HelpItemCollector &HelpItems)
{
	String FlagName = GetArgument(State, "Name");
	String FlagDescription = GetOptionalArgument(State, "Description");
	bool HasValue = GetFlag(State, "HasValue");
	HelpItems.Add(FlagName + (HasValue ? "(=VALUE)\n" : ""), MemoryStream() << "\tEnables flag \"" << FlagName << "\"" << (HasValue ? " (and sets the value to VALUE)." : "") << "  " << FlagDescription);
}

void Flag::Respond(Script &State)
{
	String FlagName = GetFlagName(Arguments);
	std::pair<bool, String> Flag = FindProgramArgument(FlagName);
	State.PushBoolean(Flag.first);
	State.PutElement("Present");
	if (Flag.first && !Flag.second.empty())
	{
		State.PushString(Flag.second);
		State.PutElement("Value");
	}
}

