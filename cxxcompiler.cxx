#include "cxxcompiler.h"

String CXXCompiler::GetIdentifier(void) { return "c++-compiler"; }

void CXXCompiler::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out) 
{
	Set<String> Flags;
	while (!Arguments.empty())
	{
		Flags.And(Arguments.front());
		Arguments.pop();
	}

	Out << "\t" << GetIdentifier() << "=PATH\n"
		"Override the detected C++ compiler.";
	if (Flags.Contains("c++11"))
		Out << "  The compiler must support C++11.";
	Out << "\n\n";
}

void CXXCompiler::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	static const std::vector<String> CompilerNames{"g++", 
	std::pair<bool, String> FoundCompiler = FindProgramArgument(GetIdentifier());
	if (!OverrideCompiler.first)
	{
		String FullPATH = getenv("PATH");
#ifdef _WIN32
		std::queue<String> PathParts = SplitString(FullPATH, {';'}, true);
#else
		std::queue<String> PathParts = SplitString(FullPATH, {':'}, true);
#endif
		while (!PathParts.empty())
		{
			for (auto CompilerName : Compilers
			if (FilePath
	}
	std::cerr << "Warning: The controller made a blank request.  Ignoring..." << std::endl; 
}


