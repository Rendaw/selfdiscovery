#include "version.h"

#include <limits>

#include "../shared.h"

typedef unsigned int VersionType;
VersionType const CurrentVersion = 1;

String Version::GetIdentifier(void) { return "Version"; }

void Version::DisplayControllerHelp(void)
{
	StandardStream << "\tDiscover." << GetIdentifier() << "{ [Version = VERSION] }\n"
		"\tResult: {Version = CURRENTVERSION}\n"
		"\tIf VERSION is specified, asserts that CURRENTVERSION is compatible with VERSION.  If the controller specified version is incompatible, this program aborts.  Returns the current version of this program as CURRENTVERSION.  This may change the behavior of the program to improve compatibility.\n\n";
}

void Version::Respond(Script &State, HelpItemCollector *)
{
	if (State.TryElement("Version"))
	{
		State.AssertNumber("Version must be a number.");
		VersionType InputVersion = State.GetUnsignedInteger();
		if (InputVersion > CurrentVersion)
			throw InteractionError("The control script requires version " + AsString(InputVersion) + " which is newer than this version (" + AsString(CurrentVersion) + ").  Please upgrade this program to continue.");
	}

	State.PushTable();
	State.PushInteger(CurrentVersion);
	State.PutElement("Version");
}

