#include "program.h"

#include <cstdlib>

#include "shared.h"

extern bool Verbose;

String Program::GetIdentifier(void) { return "program"; }

void Program::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " NAME\n"
		"\tResult: LOCATION\n"
		"\tLocates program NAME and returns the LOCATION.\n"
		"\n";
}

void Program::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out) 
{
	String ProgramName = GetNextArgument(Arguments, "program name");
	Out << "\t" << GetIdentifier() << "-" << ProgramName << "=LOCATION\n"
		"\tOverride the detected location of " << ProgramName << " with the program at LOCATION.  The executable names do not have to match, so substituting cl.exe for gcc, for instance, would not be rejected.\n"
		"\n";
}

static std::vector<DirectoryPath> GetPathParts(void)
{
	std::vector<DirectoryPath> Out;
	String FullPATH = getenv("PATH");
#ifdef _WIN32
	std::queue<String> PathParts = SplitString(FullPATH, {';'}, true);
#else
	std::queue<String> PathParts = SplitString(FullPATH, {':'}, true);
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
		std::cout << "Directories checked when searching for installed programs:\n";
		for (auto &Path : Paths)
			std::cout << "\t" << Path << "\n";
	}
}

void Program::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	String ProgramName = GetNextArgument(Arguments, "program name");
	Set<String> Flags;
	while (!Arguments.empty())
	{
		Flags.And(Arguments.front());
		Arguments.pop();
	}

	FilePath *Found = FindProgram(ProgramName);
	if ((Found == nullptr) && !Flags.Contains("optional"))
		throw InteractionError("Failed to find required program " + ProgramName);
	
	if (Found != nullptr) Out << Found->AsAbsoluteString();
	Out << "\n";
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
			FilePath OverridePath(FilePath::Qualify(OverrideProgram.second));
			if (OverridePath.Exists())
			{
				if (Verbose) std::cout << "Found program " << ProgramName << " at user-specified location " << OverridePath << std::endl;
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
					if (Verbose) std::cout << "Found program " << ProgramName << " at " << NextFilePath << std::endl;
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

