#include "cxxcompiler.h"

#include "../ren-general/arrangement.h"
#include "../ren-general/filesystem.h"

#include "../shared.h"
#include "../configuration.h"
#include "../subprocess.h"
#include "program.h"

extern Information::AnchorImplementation<Program> ProgramInformation;
extern bool Verbose;

namespace SupportFlags
{
	String const Generation2011("CXX11");
}

namespace Compilers
{
	String const GXX("g++");
	String const Clang("clang");
	String const CLEXE("cl.exe");
}

String PrintCompilerClasses(void)
{
	return MemoryStream() << Compilers::GXX << ", " << Compilers::Clang << ", " << Compilers::CLEXE;
}

static bool CompileExample(FilePath const &CompilerPath, String const &Example, std::vector<String> Arguments)
{
	auto TestFile = CreateTemporaryFile(LocateTemporaryDirectory());
	std::get<1>(TestFile) << Example << "\n" << OutputStream::Flush();
	Arguments.push_back(std::get<0>(TestFile).AsAbsoluteString());
	Subprocess Compiler(CompilerPath.AsAbsoluteString(), Arguments);
	if (Verbose)
		while (!Compiler.In.HasFailed())
			StandardStream << "Compiler output: " << Compiler.In.ReadLine() << "\n" << OutputStream::Flush();
	else Compiler.In.ReadToEnd();
	std::get<0>(TestFile).Delete();
	return Compiler.GetResult() == 0;
}

String CXXCompiler::GetIdentifier(void) { return "CXXCompiler"; }

void CXXCompiler::DisplayControllerHelp(void)
{
	StandardStream << "\tDiscover." << GetIdentifier() << "{FLAGS...}\n"
		"\tResult: {Name = COMPILER, Path = PATH}\n"
		"\tLocates a C++ compiler, returning the name in COMPILER and the full path, including executable, in PATH.  FLAGS is a space-separated list of flags that specify requirements for the compiler.\n"
		"\tFLAGS can include: " << SupportFlags::Generation2011 << "\n"
		"\tIf the compiler is recognized, COMPILER will be one of: " << PrintCompilerClasses() << "\n"
		"\n";
}

void CXXCompiler::DisplayUserHelp(Script &State, HelpItemCollector &HelpItems)
{
	HelpItems.Add(GetIdentifier() + "=PATH", String("Override the detected C++ compiler.") + (GetFlag(State, SupportFlags::Generation2011) ? "  The compiler must support C++11." : ""));
	HelpItems.Add(GetIdentifier() + "Class=CLASS", String("Override the detected C++ compiler class.  Standard values of CLASS are: " + PrintCompilerClasses()));
}

String const CXX11Example = "#include <functional>\nint main(int argc, char **argv) { std::function<void(void)> a; return 0; }";

void CXXCompiler::Respond(Script &State)
{
	bool RequireCXX11 = GetFlag(State, SupportFlags::Generation2011);
	ClearArguments(State);

	State.PushTable();
	
	std::pair<bool, String> OverrideClass = FindConfiguration(GetIdentifier() + "Class");

	auto TestCompiler = [&](FilePath const &Compiler) -> bool
	{
		if (Verbose) StandardStream << "Testing compiler \"" << Compiler << "\".\n" << OutputStream::Flush();
		String const CompilerFile = OverrideClass.first ? OverrideClass.second : Compiler.File();
		String Candidate;
		if ((CompilerFile == (Candidate = Compilers::GXX)) ||
			(CompilerFile.find("g++") != String::npos))
		{
			if (RequireCXX11)
			{
				if (Verbose) StandardStream << "Testing compiler for C++11 support.\n" << OutputStream::Flush();
				if (!CompileExample(Compiler, CXX11Example, {"-x", "c++", "-fsyntax-only", "-std=c++11"}) &&
					!CompileExample(Compiler, CXX11Example, {"-x", "c++", "-fsyntax-only", "-std=c++0x"}))
				{
					if (Verbose) StandardStream << "Compiler doesn't seem to support C++11.\n" << OutputStream::Flush();
					return false;
				}
			}
		}
		else 
		{
			if (!Compiler.Exists())
			{
				if (Verbose) StandardStream << "Overridden compiler doesn't seem to exist.\n" << OutputStream::Flush();
				return false;
			}
			Candidate = CompilerFile;
		}

		if (Verbose) StandardStream << "Compiler passed all tests.\n" << OutputStream::Flush();

		State.PushString(Candidate);
		State.PutElement("Name");
		State.PushString(Compiler.AsAbsoluteString());
		State.PutElement("Path");
		return true;
	};

	std::pair<bool, String> OverrideCompiler = FindConfiguration(GetIdentifier());
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

