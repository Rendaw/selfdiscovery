#include "clibrary.h"

#include "../shared.h"
#include "../configuration.h"
#include "platform.h"

extern bool Verbose;
extern Information::AnchorImplementation<Platform> PlatformInformation;

String CLibrary::GetIdentifier(void) { return "CLibrary"; }

void CLibrary::DisplayControllerHelp()
{
	StandardStream << "\tDiscover." << GetIdentifier() << "{Name = NAME | {NAME...}, FLAGS...}\n"
		"\tReturns: {Filename = FILENAME, LibraryDirectory = LIBRARYDIR, IncludeDirectory = INCLUDEDIR}\n"
		"\tLocates and returns information about C library NAME.  FLAGS can be any number of the following boolean flags: Optional, Static.  If Optional is not specified, the configuration will abort if the library is not found.  If Optional is specified and the library is not found, no response will be returned.  If Static is specified, a static library will be located instead of a dynamic library.  FILENAME is the name of the library file.  INCLUDEDIR will contain the location of headers associated with the library.  LIBRARYDIR will contain the location of the library itself.\n"
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

	State.PushTable();

	std::pair<bool, String> OverrideLibrary = FindConfiguration(GetIdentifier() + "-" + LibraryName),
		OverrideIncludes = FindConfiguration(GetIdentifier() + "-" + LibraryName + "-Includes");

	if (Verbose && !OverrideLibrary.first) StandardStream << "Override location for library \"" << LibraryName << "\" not specified with flag \"" << GetIdentifier() + "-" + LibraryName << "\"; proceeding with normal discovery." << "\n" << OutputStream::Flush();

	auto WriteIncludeLocation = [&](FilePath const &LibraryPath)
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
			State.PushString(ResultPath);
		}
		State.PutElement("IncludeDirectory");
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
			State.PushString(OverrideLibraryPath.File());
			State.PutElement("Filename");
			State.PushString(OverrideLibraryPath.Directory());
			State.PutElement("LibraryDirectory");
			WriteIncludeLocation(OverrideLibraryPath);
		}
		catch (Error::Construction &Failure)
		{
			throw InteractionError("The location of library \"" + LibraryName + "\" was manually specified but rejected because: " + Failure.Explanation);
		}
		return;
	}
	else
	{
		auto ProcessLibraryLocation = [&](FilePath const &Location) -> bool
		{
			if (Verbose) StandardStream << "Testing for library \"" << LibraryName << "\" at \"" << Location << "\"\n" << OutputStream::Flush();
			if (!Location.Exists()) return false;
			State.PushString(Location.File());
			State.PutElement("Filename");
			State.PushString(Location.Directory());
			State.PutElement("LibraryDirectory");
			WriteIncludeLocation(Location);
			return true;
		};

		for (auto &TestName : LibraryNames)
		{
			for (auto &TestLocation : TestLocations)
			{
				if (ProcessLibraryLocation(TestLocation.Select(TestName))) return;

				if (PlatformInformation->GetFamily() == Platform::Families::Windows)
				{
					if (RequireStatic)
						{ if (ProcessLibraryLocation(TestLocation.Select(TestName + ".lib"))) return; }
					else if (ProcessLibraryLocation(TestLocation.Select(TestName + ".dll"))) return;
				}
				else
				{
					if (RequireStatic)
					{
						if (ProcessLibraryLocation(TestLocation.Select("lib" + TestName + ".a"))) return;
						if (ProcessLibraryLocation(TestLocation.Select(TestName + ".a"))) return;
					}
					else
					{
						if (ProcessLibraryLocation(TestLocation.Select("lib" + TestName + ".so"))) return;
						if (ProcessLibraryLocation(TestLocation.Select(TestName + ".so"))) return;
					}
				}
			}
		}
	}

	if (!Optional)
		throw InteractionError("Could not find required library " + LibraryName + ".  If you believe you have the library, check the help and specify the correct location on the command line.");
}

