#include "program.h"

#include <cstdlib>

#include "../shared.h"
#include "../configuration.h"

extern bool Verbose;

String Program::GetIdentifier(void) { return "Program"; }

void Program::DisplayControllerHelp(void)
{
	StandardStream << "\tDiscover." << GetIdentifier() << "{Name = NAME, FLAGS...}\n"
		"\tResult: {Location = LOCATION}\n"
		"\tLocates program NAME and returns the LOCATION.  If FLAGS contains Optional, failure to locate the program will not terminate discovery, but nil will be returned rather than a table.\n"
		"\n";
}

void Program::DisplayUserHelp(Script &State, HelpItemCollector &HelpItems) 
{
	String ProgramName = GetArgument(State, "Name");
	HelpItems.Add(GetIdentifier() + "-" + ProgramName + "=LOCATION", "Override the detected location of " + ProgramName + " with the program at LOCATION.  The executable names do not have to match, so substituting cl.exe for gcc, for instance, would not be rejected.");
}

static std::vector<DirectoryPath> GetPathParts(void)
{
	std::vector<DirectoryPath> Out;
	String FullPATH = getenv("PATH");
#ifdef _WIN32
	std::queue<String> PathParts = StringSplitter({';'}, true).Process(FullPATH).Results();
#else
	std::queue<String> PathParts = StringSplitter({':'}, true).Process(FullPATH).Results();
#endif
	while (!PathParts.empty())
	{
		Out.push_back(DirectoryPath::Qualify(PathParts.front()));
		PathParts.pop();
	}
	return std::move(Out);
}

Program::Program(void) : Paths(GetPathParts()) 
{
	if (Verbose)
	{
		StandardStream << "The following directories will be checked when searching for installed programs:\n";
		for (auto &Path : Paths)
			StandardStream << "\t" << Path << "\n";
	}
}

void Program::Respond(Script &State)
{
	String ProgramName = GetArgument(State, "Name");


	std::pair<bool, String> Override = FindConfiguration(GetIdentifier() + "-" + ProgramName);
	if (Override.first)
	{
		if (Verbose)
			StandardStream << "Found program \"" + ProgramName + "\" by configuration.\n";
		State.PushTable();
		State.PushString(Override.second);
		State.PutElement("Location");
		return;
	}

	FilePath *Found = FindProgram(ProgramName);
	if ((Found == nullptr) && !GetFlag(State, "Optional"))
		throw InteractionError("Failed to find required program \"" + ProgramName + "\"");
	
	State.PushTable();
	if (Found != nullptr) 
	{
		State.PushString(Found->AsAbsoluteString());
		State.PutElement("Location");
	}
}

FilePath *Program::FindProgram(String const &ProgramName)
{
	// Check if we've already located a program
	auto FoundProgram = Programs.find(ProgramName);
	if (FoundProgram == Programs.end()) 
	{
		// Check to see if the user's explicitly set the program's location
		std::pair<bool, String> OverrideProgram = FindConfiguration(GetIdentifier() + "-" + ProgramName);
		if (OverrideProgram.first)
		{
			FilePath OverridePath(FilePath::Qualify(OverrideProgram.second));
			if (OverridePath.Exists())
			{
				if (Verbose) StandardStream << "Found program \"" << ProgramName << "\" at user-specified location " << OverridePath << "\n" << OutputStream::Flush();
				Programs[ProgramName] = new FilePath(OverridePath);
			}
		}
		else
		{
			// Do a search, directory-by-directory, through environment variable PATH
			for (auto &NextPath : Paths)
			{
				FilePath NextFilePath = NextPath.Select(ProgramName);
				if (NextFilePath.Exists())
				{
					if (Verbose) StandardStream << "Found program \"" << ProgramName << "\" at \"" << NextFilePath << "\".\n" << OutputStream::Flush();
					Programs[ProgramName] = new FilePath(NextFilePath);
					break;
				}
			}
		}
		FoundProgram = Programs.find(ProgramName);
	}
	if (FoundProgram == Programs.end())
		return nullptr;
	return FoundProgram->second;
}

