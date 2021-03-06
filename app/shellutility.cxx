#include "shellutility.h"

#include <cstring>

#include "ren-general/inputoutput.h"
#include "ren-general/filesystem.h"

void ShowShellUtilityHelp(void)
{
	StandardStream << "\tUtility.MakeDirectory{Directory = DIRECTORY [, FLAGS...]}\n"
		"\tReturns: Nothing\n"
		"\tCreates a directory, and, optionally, all the missing directories above that directory.  If this operation fails, it will silently abort.\n"
		"\n"
		"\tUtility.Call{Command = COMMAND [, WorkingDirectory = DIRECTORY]}\n"
		"\tReturns: STATUS\n"
		"\tRuns command COMMAND in working directory DIRECTORY or the current working directory if not specified.  Returns the exit code when complete.\n"
		"\n";
}


void RegisterShellUtilities(Script &State)
{
	State.PushFunction([](Script &State) -> int
	{
		State.AssertTable("MakeDirectory requires arguments be passed in a table.");
		State.PullElement("Directory");
		State.AssertString("Invalid or missing \"Directory\" argument.");
		String Directory = State.GetString();
		bool MakeParents = false;
		if (State.TryElement("MakeParents"))
		{
			State.AssertBoolean("Invalid boolean \"MakeParents\" argument.");
			MakeParents = State.GetBoolean();
		}
		DirectoryPath::Qualify(Directory).Create(MakeParents);

		return 0;
	});
	State.PutElement("MakeDirectory");

	State.PushFunction([](Script &State) -> int
	{
		State.AssertTable("Call requires arguments be passed in a table.");
		State.PullElement("Command");
		State.AssertString("Invalid or missing \"Command\" argument.");
		String Command = State.GetString();

		DirectoryPath InitialDirectory = LocateWorkingDirectory();
		if (State.TryElement("WorkingDirectory"))
		{
			State.AssertString("Invalid \"WorkingDirectory\" argument.");
			ChangeWorkingDirectory(DirectoryPath::Qualify(State.GetString()));
		}

#ifdef WINDOWS
		int Result = _wsystem((wchar_t const *)AsNativeString(Command).c_str());
#else
		int Result = system(Command.c_str());

		// For some reason on windows system always returns -1 even when it work.  If this changes, make this code shared
		if (Result == -1)
			throw Error::System(String("Internal error while trying to call command: ") + strerror(errno));
#endif
#ifdef WINDOWS
		State.PushInteger(Result);
#else
		State.PushInteger(WEXITSTATUS(Result));
#endif
		
		ChangeWorkingDirectory(InitialDirectory);
		return 1;
	});
	State.PutElement("Call");
}

