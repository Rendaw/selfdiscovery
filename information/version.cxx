#include "version.h"

#include <limits>

#include "../shared.h"

typedef unsigned int VersionType;
VersionType const CurrentVersion = 0;

String Version::GetIdentifier(void) { return "version"; }

void Version::DisplayControllerHelp(void)
{
	StandardStream << "\tDiscover." << GetIdentifier() << "{(Version = VERSION)}\n"
		"\tResult: {Version = CURRENTVERSION}\n"
		"\tIf VERSION is specified, asserts that CURRENTVERSION is compatible with VERSION.  If the controller specified version is incompatible, this program aborts.  Returns the current version of this program as CURRENTVERSION.  This may change the behavior of the program to improve compatibility.\n";
}

void Version::DisplayUserHelp(Script &State, HelpItemCollector &HelpItems) {}

void Version::Respond(Script &State)
{
	if (State.TryElement("Version"))
	{
		State.AssertNumber("Version must be a number.");
		VersionType InputVersion = State.GetUnsignedInteger();
		if (InputVersion > CurrentVersion)
			throw InteractionError("The control script requires version " + AsString(InputVersion) + " which is newer than this version (" + AsString(CurrentVersion) + ").  Please upgrade this program to continue.");
	}

	State.PushInteger(CurrentVersion);
	State.PutElement("Version");
}

