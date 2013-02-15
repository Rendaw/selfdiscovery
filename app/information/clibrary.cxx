#include "clibrary.h"

#include "../shared.h"
#include "../configuration.h"
#include "../subprocess.h"
#include "platform.h"
#include "program.h"

extern bool Verbose;
extern Information::AnchorImplementation<Platform> PlatformInformation;
extern Information::AnchorImplementation<Program> ProgramInformation;

String CLibrary::GetIdentifier(void) { return "CLibrary"; }

void CLibrary::DisplayControllerHelp()
{
	StandardStream << "\tDiscover." << GetIdentifier() << "{Name = NAME | {NAME...}, FLAGS...}\n"
		"\tReturns: {Filenames = {FILENAME...}, LibraryDirectories = {LIBRARYDIR...}, IncludeDirectories = {INCLUDEDIR...} }\n"
		"\tLocates and returns information about C library NAME.  If NAME is plural, the values of NAME will be searched for in order until one is found.\n"
		"\tFLAGS can be any number of the following boolean flags: Optional, Static.  If Optional is not specified, the configuration will abort if the library is not found.  If Optional is specified and the library is not found, no response will be returned.  If Static is specified, a static library will be located instead of a dynamic library.\n"
		"\tFILENAME is the filename of the library, multiple names if the library has multiple files.  If pkg-config is used to locate the library, FILENAME might be a ld-style library namespec.  INCLUDEDIR will contain the location of headers associated with the library.  LIBRARYDIR will contain the location of the library itself.  NAME, LIBRARYDIR, and INCLUDEDIR will generally be singular and LIBRARYDIR will contain the file specified by NAME, except when using pkg-config.\n"
		"\n";
}

void CLibrary::DisplayUserHelp(Script &State, HelpItemCollector &HelpItems)
{
	String LibraryName = GetArgument(State, "Name");
	HelpItems.Add(GetIdentifier() + "-" + LibraryName + "=LOCATION",
		MemoryStream() << "Overrides the location of library " << LibraryName << ".  This path should include both the absolute directory and the filename.  Unless the include location is separately overridden, the include path will be deduced from LOCATION.");
	HelpItems.Add(GetIdentifier() + "-" + LibraryName + "-Includes=LOCATION", "Overrides the location of include files for library " + LibraryName + ".");
}

static std::vector<DirectoryPath> SplitEnvironmentVariableParts(String const &Raw)
{
	std::vector<DirectoryPath> Out;
	std::queue<String> Parts = StringSplitter({':'}, true).Process(Raw).Results();
	while (!Parts.empty())
	{
		Out.push_back(DirectoryPath::Qualify(Parts.front()));
		Parts.pop();
	}
	return std::move(Out);
}

static std::vector<DirectoryPath> GatherTestLocations(void)
{
	std::vector<DirectoryPath> TestLocations;

	if (PlatformInformation->GetArchitectureBits() == 64)
	{
		// If on 64, gather LD_LIBRARY64_PATH locations
		char const *LD64Path = getenv("LD_LIBRARY64_PATH");
		if (LD64Path != nullptr)
		{
			auto NewParts = SplitEnvironmentVariableParts(LD64Path);
			TestLocations.insert(TestLocations.end(), NewParts.begin(), NewParts.end());
		}
	}
	else if (PlatformInformation->GetArchitectureBits() == 32)
	{
		// If on 32, try LD_LIBRARY32_PATH values
		char const *LD32Path = getenv("LD_LIBRARY32_PATH");
		if (LD32Path != nullptr)
		{
			auto NewParts = SplitEnvironmentVariableParts(LD32Path);
			TestLocations.insert(TestLocations.end(), NewParts.begin(), NewParts.end());
		}
	}

	// Try LD_LIBRARY_PATH values
	char const *LD32Path = getenv("LD_LIBRARY32_PATH");
	if (LD32Path != nullptr)
	{
		auto NewParts = SplitEnvironmentVariableParts(LD32Path);
		TestLocations.insert(TestLocations.end(), NewParts.begin(), NewParts.end());
	}

	// Try system directories
#ifdef _WIN32
#else
	for (auto &PlatformBase : std::vector<String>({"", "/usr", "/usr/local", "/opt"}))
	{
		if (PlatformInformation->GetMember() == Platform::Members::LinuxUbuntu)
		{
			if (PlatformInformation->GetArchitectureBits() == 64) 
				TestLocations.push_back(PlatformBase + "/lib/x86_64-linux-gnu");
		}

		if (PlatformInformation->GetLinuxClass() == Platform::LinuxClasses::Debian)
		{
			if (PlatformInformation->GetArchitectureBits() == 64) 
				TestLocations.push_back(PlatformBase + "/lib");
			else TestLocations.push_back(PlatformBase + "/lib32");
		}
		else if (PlatformInformation->GetLinuxClass() == Platform::LinuxClasses::RedHat)
		{
			if (PlatformInformation->GetArchitectureBits() == 64) 
				TestLocations.push_back(PlatformBase + "/lib64");
			else TestLocations.push_back(PlatformBase + "/lib");
		}
		else TestLocations.push_back(PlatformBase + "/lib");
	}
#endif

	return std::move(TestLocations);
}

CLibrary::CLibrary(void) : TestLocations(GatherTestLocations()) 
{
	if (Verbose)
	{
		StandardStream << "Checking the following directories for libraries:\n";
		for (auto &Location : TestLocations)
			StandardStream << "\t" << Location << "\n";
		StandardStream << OutputStream::Flush();
	}
}

void CLibrary::Respond(Script &State)
{
	std::vector<String> LibraryNames = GetVariableArgument(State, "Name");
	bool RequireStatic = GetFlag(State, "Static");
	bool Optional = GetFlag(State, "Optional");
	ClearArguments(State);

	String const LibraryName = LibraryNames[0];

	std::pair<bool, String> OverrideLibrary = FindConfiguration(GetIdentifier() + "-" + LibraryName),
		OverrideIncludes = FindConfiguration(GetIdentifier() + "-" + LibraryName + "-Includes");

	if (Verbose && !OverrideLibrary.first) StandardStream << "Override location for library \"" << LibraryName << "\" not specified with flag \"" << GetIdentifier() + "-" + LibraryName << "\"; proceeding with normal discovery." << "\n" << OutputStream::Flush();

	// Results
	bool Found = false;
	std::vector<String> LibraryFilenames;
	std::vector<String> LibraryLocations;
	std::vector<String> IncludeLocations;

	auto AddLibraryFilename = [&](String const Filename)
	{
		Found = true;
		LibraryFilenames.push_back(Filename);
	};
	
	auto AddLibraryLocation = [&](String const Location)
	{
		Found = true;
		LibraryLocations.push_back(Location);
	};
	
	auto AddIncludeLocation = [&](String const Location)
	{
		Found = true;
		IncludeLocations.push_back(Location);
	};

	auto FindIncludeLocation = [&](FilePath const &LibraryPath)
	{
		if (OverrideIncludes.first) State.PushString(OverrideIncludes.second);
		else 
		{
			DirectoryPath ResultPath = LibraryPath.Directory();
			DirectoryPath IncludeSearchPath = ResultPath;
			while (!IncludeSearchPath.IsRoot())
			{
				IncludeSearchPath = IncludeSearchPath.Exit();
				if (IncludeSearchPath.Select("include").Exists())
				{
					ResultPath = IncludeSearchPath.Enter("include");
					break;
				}
			}
			AddIncludeLocation(ResultPath);
		}
	};

	if (OverrideLibrary.first)
	{
		// Try override values
		try
		{
			FilePath OverrideLibraryPath = FilePath::Qualify(OverrideLibrary.second);
			if (Verbose) StandardStream << "Testing for library \"" << LibraryName << "\" at \"" << OverrideLibraryPath << "\"\n" << OutputStream::Flush();
			if (!OverrideLibraryPath.Exists())
				throw InteractionError("The location of library \"" + LibraryName + "\" was manually specified but the file does not exist at that location.");
			AddLibraryFilename(OverrideLibraryPath.File());
			AddLibraryLocation(OverrideLibraryPath.Directory());
			FindIncludeLocation(OverrideLibraryPath);
		}
		catch (Error::Construction &Failure)
		{
			throw InteractionError("The location of library \"" + LibraryName + "\" was manually specified but rejected because: " + Failure.Explanation);
		}
	}
	
	if (!Found)
	{
		// Do a brute force search
		auto ProcessLibraryLocation = [&](FilePath const &Location) -> bool
		{
			if (Verbose) StandardStream << "Testing for library \"" << LibraryName << "\" at \"" << Location << "\"\n" << OutputStream::Flush();
			if (!Location.Exists()) return false;
			AddLibraryFilename(Location.File());
			AddLibraryLocation(Location.Directory());
			FindIncludeLocation(Location);
			return true;
		};

		for (auto &TestName : LibraryNames)
		{
			for (auto &TestLocation : TestLocations)
			{
				if (ProcessLibraryLocation(TestLocation.Select(TestName))) break;

				if (PlatformInformation->GetFamily() == Platform::Families::Windows)
				{
					if (RequireStatic)
						{ if (ProcessLibraryLocation(TestLocation.Select(TestName + ".lib"))) break; }
					else if (ProcessLibraryLocation(TestLocation.Select(TestName + ".dll"))) break;
				}
				else
				{
					if (RequireStatic)
					{
						if (ProcessLibraryLocation(TestLocation.Select("lib" + TestName + ".a"))) break;
						if (ProcessLibraryLocation(TestLocation.Select(TestName + ".a"))) break;
					}
					else
					{
						if (ProcessLibraryLocation(TestLocation.Select("lib" + TestName + ".so"))) break;
						if (ProcessLibraryLocation(TestLocation.Select(TestName + ".so"))) break;
					}
				}
			}
			if (Found) break;
		}
	}

	if (!Found)
	{
		// Try pkg-config, because GTK has (multiple) inconsistently named binaries
		FilePath *PkgConfigPath = ProgramInformation->FindProgram("pkg-config");
#ifdef WINDOWS
		if (PkgConfigPath == nullptr) PkgConfigPath = ProgramInformation->FindProgram("pkg-config.exe");
		if (PkgConfigPath == nullptr) PkgConfigPath = ProgramInformation->FindProgram("pkg-config.EXE");
#endif
		if (PkgConfigPath != nullptr)
		{
			for (auto &TestName : LibraryNames)
			{
				Subprocess IncludeFinder(PkgConfigPath->AsAbsoluteString(), {"--cflags", TestName});
				StringSplitter IncludeSplits({' '}, true);
				while (!IncludeFinder.Out.HasFailed())
				{
					IncludeSplits.Process(IncludeFinder.Out.ReadLine());
					if (Verbose)
						StandardStream << "Include pkg-config output: " << IncludeFinder.Out.ReadLine() << "\n" << OutputStream::Flush();
				}
				
				Subprocess LibraryFinder(PkgConfigPath->AsAbsoluteString(), {"--libs", TestName});
				StringSplitter LibrarySplits({' '}, true);
				while (!LibraryFinder.Out.HasFailed())
				{
					LibrarySplits.Process(LibraryFinder.Out.ReadLine());
					if (Verbose)
						StandardStream << "Library pkg-config output: " << LibraryFinder.Out.ReadLine() << "\n" << OutputStream::Flush();
				}

				if ((IncludeFinder.GetResult() != 0) || 
					(LibraryFinder.GetResult() != 0)) continue;

				{
					auto Results = IncludeSplits.Results();
					while (!Results.empty())
					{
						auto Result = Results.front();
						if (Result.substr(0, 2) == "-I")
							AddIncludeLocation(Result.substr(2));
						Results.pop();
					}
				}
				
				{
					auto Results = LibrarySplits.Results();
					while (!Results.empty())
					{
						auto Result = Results.front();
						if (Result.substr(0, 2) == "-L")
							AddLibraryLocation(Result.substr(2));
						if (Result.substr(0, 2) == "-l")
							AddLibraryFilename(Result.substr(2));
						Results.pop();
					}
				}

				if (Found) break;
			}
		}
	}

	if (!Found && !Optional)
		throw InteractionError("Could not find required library " + LibraryName + ".  If you believe you have the library, check the help and specify the correct location on the command line.");

	assert(State.Height() == 0);
	State.PushTable();

	State.PushTable();
	for (unsigned int Index = 1; Index <= LibraryFilenames.size(); ++Index) 
	{
		State.PushString(LibraryFilenames[Index - 1]);
		State.PutElement(Index);
	}
	State.PutElement("Filenames");
	
	State.PushTable();
	for (unsigned int Index = 1; Index <= LibraryLocations.size(); ++Index) 
	{
		State.PushString(LibraryLocations[Index - 1]);
		State.PutElement(Index);
	}
	State.PutElement("LibraryDirectories");
	
	State.PushTable();
	for (unsigned int Index = 1; Index <= IncludeLocations.size(); ++Index) 
	{
		State.PushString(IncludeLocations[Index - 1]);
		State.PutElement(Index);
	}
	State.PutElement("IncludeDirectories");
	assert(State.Height() == 1);
}

