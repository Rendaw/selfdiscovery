#include "version.h"

#include <limits>

#include "shared.h"

typedef unsigned int VersionType;
VersionType const CurrentVersion = 0;

String Version::GetIdentifier(void) { return "version"; }

void Version::DisplayControllerHelp(OutputStream &Out)
{
	Out << "\t" << GetIdentifier() << " (VERSION)\n"
		"\tResult: CURRENTVERSION\n"
		"\tIf VERSION is specified, asserts that CURRENTVERSION is compatible with VERSION.  If the controller specified version is incompatible, this program aborts.  Returns the current version of this program as CURRENTVERSION.  This may cause information items to behave differently to ensure compatibility.\n";
}

void Version::DisplayUserHelp(std::queue<String> &&Arguments, OutputStream &Out) { }

void Version::Respond(std::queue<String> &&Arguments, OutputStream &Out)
{
	if (!Arguments.empty())
	{
		MemoryStream InputStream(Arguments.front());
		VersionType InputVersion = std::numeric_limits<VersionType>::max();
		String InputVersionString;
		InputStream >> InputVersionString;
		std::stringstream Convert(InputVersionString);
		Convert >> InputVersion;
		if (InputVersion > CurrentVersion)
			throw InteractionError("The control script requires version " + Arguments.front() + " which is newer than this version (" + AsString(CurrentVersion) + ").  Please upgrade this program to continue.");
	}

	Out << CurrentVersion << "\n";
}

