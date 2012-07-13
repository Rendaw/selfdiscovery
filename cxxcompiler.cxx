#include "cxxcompiler.h"

#include "ren-general/arrangement.h"
#include "ren-general/filesystem.h"

#include "shared.h"
#include "program.h"

extern Information::AnchorImplementation<Program> ProgramInformation;
extern bool Verbose;

namespace SupportFlags
{
	String const Generation2011("c++11");
}

namespace Compilers
{
	String const GXX("g++");
	String const Clang("clang");
	String const CLEXE("cl.exe");
}

static bool CompileExample(FilePath const &CompilerPath, String const &Example, std::vector<String> Arguments)
{
	FileOutput TestFile;
	FilePath TestFileLocation = CreateTemporaryFile(LocateTemporaryDirectory(), TestFile);
	TestFile << Example << std::endl;
	Arguments.push_back(TestFileLocation.AsAbsoluteString());
	Subprocess Compiler(CompilerPath.AsAbsoluteString(), Arguments);
	if (Verbose)
		while (!Compiler.In.HasFailed())
			std::cout << "Compiler output: " << Compiler.In.ReadLine() << std::endl;
	else Compiler.In.ReadToEnd();
	TestFileLocation.Delete();
	return Compiler.GetResult() == 0;
}

String CXXCompiler::GetIdentifier(void) { return "c++-compiler"; }

void CXXCompiler::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " (" << SupportFlags::Generation2011 << ")\n"
		"\tResult: COMPILER PATH\n"
		"\tLocates a C++ compiler, returning the name in COMPILER and the full path, including executable, in PATH.  FLAGS is a space-separated list of flags that specify requirements for the compiler.\n"
		"\tCOMPILER will be one of: " << Compilers::GXX << ", " << Compilers::Clang << ", " << Compilers::CLEXE << "\n"
		"\tFLAGS can include: " << SupportFlags::Generation2011 << "\n"
		"\n";
}

void CXXCompiler::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out) 
{
	Set<String> Flags;
	while (!Arguments.empty())
	{
		Flags.And(Arguments.front());
		Arguments.pop();
	}

	Out << "\t" << GetIdentifier() << "=PATH\n"
		"\tOverride the detected C++ compiler.";
	if (Flags.Contains(SupportFlags::Generation2011))
		Out << "  The compiler must support C++11.";
	Out << "\n\n";
}

String const CXX11Example = "#include <functional>\nint main(int argc, char **argv) { std::function<void(void)> a; return 0; }";

void CXXCompiler::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	Set<String> Flags;
	while (!Arguments.empty())
	{
		Flags.And(Arguments.front());
		Arguments.pop();
	}

	auto TestCompiler = [&](FilePath const &Compiler) -> bool
	{
		if (Verbose) std::cout << "Testing compiler " << Compiler << ".\n";
		String const CompilerFile = Compiler.File();
		String Candidate = Compilers::GXX;
		if ((CompilerFile == (Candidate = Compilers::GXX)) ||
			(CompilerFile.find("g++") != String::npos))
		{
			if (Flags.Contains(SupportFlags::Generation2011))
			{
				if (Verbose) std::cout << "Testing compiler for C++11 support." << std::endl;
				if (!CompileExample(Compiler, CXX11Example, {"-x", "c++", "-fsyntax-only", "-std=c++11"}) &&
					!CompileExample(Compiler, CXX11Example, {"-x", "c++", "-fsyntax-only", "-std=c++0x"}))
					return false;
			}
		}
		else return false; // Maybe unknown compilers should just be accepted, or assume they take g++-style arguments?

		Out << Candidate << " " << Compiler.AsAbsoluteString() << "\n";
		return true;
	};

	std::pair<bool, String> OverrideCompiler = FindProgramArgument(GetIdentifier());
	if ((OverrideCompiler.first) && (TestCompiler(FilePath::Qualify(OverrideCompiler.second))))
		return;
	
	for (auto &ProgramName : std::vector<String>({Compilers::GXX, Compilers::Clang, Compilers::CLEXE}))
	{
		FilePath *FoundProgram = ProgramInformation->FindProgram(ProgramName);
		if ((FoundProgram != nullptr) && (TestCompiler(*FoundProgram)))
			return;
	}

	throw InteractionError("Could not find a suitable C++ compiler!  Existing C++ compilers may not support the requested features.  Rerun this program in help mode to see the necessary features.");
}

