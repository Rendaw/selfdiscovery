#include "clibrary.h"

#include "shared.h"
#include "platform.h"

extern bool Verbose;
extern Information::AnchorImplementation<Platform> PlatformInformation;

String CLibrary::GetIdentifier(void) { return "c-library"; }

void CLibrary::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << " NAME FLAGS\n"
		"\tResult: FILENAME INCLUDEDIR LIBRARYDIR\n"
		"\tLocates and returns information about C library NAME.  FLAGS can be any number of the following flags, separated by spaces: optional, static.  If \"optional\" is not specified, the configuration will abort if the library is not found.  If \"optional\" is specified and the library is not found, no response will be returned.  If \"static\" is specified, a static library will be located instead of a dynamic library.  FILENAME is the name of the library file.  INCLUDEDIR will contain the location of headers associated with the library.  LIBRARYDIR will contain the location of the library itself.\n"
		"\n";
}

void CLibrary::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	String LibraryName = GetNextArgument(Arguments, "library name");
	Out << "\t" << GetIdentifier() << "-" << LibraryName << "=LOCATION\n"
		"\tOverrides the location of library " << LibraryName << ".  This path should include both the absolute directory and the filename.  Unless the include location is separately overridden, the include path will be deduced from LOCATION.\n\n";
	Out << "\t" << GetIdentifier() << "-" << LibraryName << "-includes=LOCATION\n"
		"\tOverrides the location of include files for library " << LibraryName << ".\n\n";
}

static std::vector<DirectoryPath> SplitEnvironmentVariableParts(String const &Raw)
{
	std::vector<DirectoryPath> Out;
	std::queue<String> Parts = SplitString(Raw, {':'}, true);
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
		std::cout << "Checking the following directories for libraries:\n";
		for (auto &Location : TestLocations)
			std::cout << "\t" << Location << "\n";
	}
}

void CLibrary::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	String LibraryName = GetNextArgument(Arguments, "library name");
	Set<String> Flags;
	while (!Arguments.empty())
	{
		Flags.And(Arguments.front());
		Arguments.pop();
	}
	std::pair<bool, String> OverrideLibrary = FindProgramArgument(GetIdentifier() + "-" + LibraryName),
		OverrideIncludes = FindProgramArgument(GetIdentifier() + "-" + LibraryName + "-includes");

	if (Verbose && !OverrideLibrary.first) std::cout << "Override location for library " << LibraryName << " not specified with flag: " << GetIdentifier() + "-" + LibraryName << ", proceeding with normal discovery." << std::endl;

	auto WriteIncludeLocation = [&](FilePath const &LibraryPath)
	{
		if (OverrideIncludes.first) Out << " " << OverrideIncludes.second;
		else Out << " " << LibraryPath.Directory().Exit().Enter("include");
	};

	if (OverrideLibrary.first)
	{
		// Try override values
		FilePath OverrideLibraryPath = OverrideLibrary.second;
		if (Verbose) std::cout << "Testing for library " << LibraryName << " at " << OverrideLibraryPath << std::endl;
		if (!OverrideLibraryPath.Exists())
			throw InteractionError("The location of library " + LibraryName + " was manually specified but the file does not exist at that location.");
		Out << OverrideLibraryPath.File() << " " << OverrideLibraryPath.Directory();
		WriteIncludeLocation(OverrideLibraryPath);
		Out << "\n";
		return;
	}
	else
	{
		auto ProcessLibraryLocation = [&](FilePath const &Location) -> bool
		{
			if (Verbose) std::cout << "Testing for library " << LibraryName << " at " << Location << std::endl;
			if (!Location.Exists()) return false;
			Out << Location.File() << " " << Location.Directory();
			WriteIncludeLocation(Location);
			Out << "\n";
			return true;
		};

		for (auto &TestLocation : TestLocations)
		{
			if (ProcessLibraryLocation(TestLocation.Select(LibraryName))) return;

			if (PlatformInformation->GetFamily() == Platform::Families::Windows)
			{
				if (Flags.Contains("static"))
					{ if (ProcessLibraryLocation(TestLocation.Select(LibraryName + ".lib"))) return; }
				else if (ProcessLibraryLocation(TestLocation.Select(LibraryName + ".dll"))) return;
			}
			else
			{
				if (Flags.Contains("static"))
				{
					if (ProcessLibraryLocation(TestLocation.Select("lib" + LibraryName + ".a"))) return;
					if (ProcessLibraryLocation(TestLocation.Select(LibraryName + ".a"))) return;
				}
				else
				{
					if (ProcessLibraryLocation(TestLocation.Select("lib" + LibraryName + ".so"))) return;
					if (ProcessLibraryLocation(TestLocation.Select(LibraryName + ".so"))) return;
				}
			}
		}
	}

	if (!Flags.Contains("optional"))
		throw InteractionError("Could not find required library " + LibraryName + ".  If you believe you have the library, check the help and specify the correct location on the command line.");
}

