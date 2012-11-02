#include "information.h"

String GetArgument(Script &State, String const &Name)
{
	State.AssertTable("Arguments must be passed to this function in a table.  It appears that you passed in a " + State.GetType() + ".");
	State.PullElement(Name);
	State.AssertString("Invalid or missing required string parameter \"" + Name + "\".");
	String Out = State.GetString();
	if (Out.empty())
		throw Error::Input("Required string parameter \"" + Name + "\" cannot be empty.");
	return Out;
}

String GetOptionalArgument(Script &State, String const &Name)
{
	State.AssertTable("Arguments must be passed to this function in a table.  It appears that you passed in a " + State.GetType() + ".");
	if (!State.TryElement(Name))
		return String();
	State.AssertString("Argument " + Name + " must be a string.");
	return State.GetString();
}

bool GetFlag(Script &State, String const &Name)
{
	State.AssertTable("Arguments must be passed to this function in a table.  It appears that you passed in a " + State.GetType() + ".");
	if (!State.TryElement(Name))
		return false;
	State.AssertBoolean("Flag " + Name + " must be a boolean.");
	return State.GetBoolean();
}

void ClearArguments(Script &State)
{
	assert(State.IsTable());
	State.Pop();
	assert(State.Height() == 0);
}
		
void HelpItemCollector::Add(String const &Argument, String const &NewDescription)
{
	(*this)[Argument];
	(*this)[Argument].And(NewDescription);
}

namespace Information
{
	Anchor::~Anchor(void) {}
}

