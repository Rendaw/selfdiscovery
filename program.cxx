#include "program.h"

String Program::GetIdentifier(void) { return "program"; }

void Program::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " NAME\n"
		"\tResult: LOCATION\n"
		"Locates program NAME and returns the LOCATION.\n"
		"\n";
}

void Program::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out) 
{
	ProgramName = GetNextArgument(Arguments, "program name");
	Out << "\t" << GetIdentifier() << "-" << ProgramName << "=LOCATION\n"
		"Override the detected location of " << ProgramName << " with the program at LOCATION.  The executable names do not have to match, so substituting cl.exe for gcc, for instance, would not be rejected."
		"\n\n";
}

static std::vector<DirectoryPath> GetPathParts(void)
{
	std::vector<String> Out;
#ifdef _WIN32
	std::queue<String> PathParts = SplitString(FullPATH, {';'}, true);
#else
	std::queue<String> PathParts = SplitString(FullPATH, {':'}, true);
#endif
	while (!PathParts.empty())
	{
		Out.push_back(PathParts.front());
		PathParts.pop();
	}
	return std::move(Out);
}

Program::Program(void) : Paths(GetPathParts()) {}

void Program::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	ProgramName = GetNextArgument(Arguments, "program name");
	Set<String> Flags;
	while (!Arguments.empty())
	{
		Flags.And(Arguments.front());
		Arguments.pop();
	}

	FilePath *Found = FindProgram(ProgramName);
	if ((Found == nullptr) && !Flags.Contains["optional"])
		throw InteractionError("Failed to find required program " + ProgramName);
	
	if (Found != nullptr) Out << Found->AsAbsolutePath();
	Out << "\n\n";
}

FilePath *Program::FindProgram(String const &ProgramName)
{
	// Check if we've already located a program
	auto FoundProgram = Programs.find(ProgramName);
	if (FoundProgram == Programs.end()) 
	{
		// Check to see if the user's explicitly set the program's location
		std::pair<bool, String> OverrideProgram = FindProgramArgument(GetIdentifier() + "-" + ProgramName);
		if (OverrideProgram.first)
		{
			FilePath OverridePath(OverrideProgram.second);
			if (OverridePath.Exists())
				Programs[ProgramName] = std::move(NextFilePath);
		}
		else
		{
			// Do a search, directory-by-directory, through environment variable PATH
			for (auto &NextPath : Paths)
			{
				FilePath NextFilePath = NextPath.Select(ProgramName);
				if (NextFilePath.Exists())
				{
					Programs[ProgramName] = std::move(NextFilePath);
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

