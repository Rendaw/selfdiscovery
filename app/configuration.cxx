#include "configuration.h"

#include "ren-general/arrangement.h"
#include "ren-general/range.h"

#include "shared.h"

std::map<String, Configuration> ProgramConfiguration;

std::pair<bool, String> FindConfiguration(String const &Name)
{
	std::map<String, Configuration>::iterator Found = ProgramConfiguration.find(Name);
	if (Found != ProgramConfiguration.end())
		return std::pair<bool, String>(true, Found->second.Value);
	return std::pair<bool, String>(false, String());
}

void LoadConfigurationFile(FilePath const &File)
{
	try 
	{
		FileInput UserOverrides = File;

		String ConfigLine;
		bool InputRemains = true;

		while (InputRemains)
		{
			StringSplitter StripWhitespace({' ', '\t'}, true);
			while (1)
			{
				if (!(UserOverrides >> ConfigLine))
				{
					InputRemains = false;
					break;
				}

				if (StripWhitespace.Process(ConfigLine).Finished())
					break;
			}

			if (StripWhitespace.Results().empty()) continue;

			StringSplitter SplitKeyValue({'='}, false);
			SplitKeyValue.Process(StripWhitespace.Results().front());
			assert(SplitKeyValue.Finished());
			assert(RangeD(1,2).Contains(SplitKeyValue.Results().size()));
			String Key = SplitKeyValue.Results().front();
			SplitKeyValue.Results().pop();
			String Value = SplitKeyValue.Results().back();
			ProgramConfiguration[Key] = {Value, File};
		}
	}
	catch (Error::System &Error) { }
}

void LoadConfigurationCommandline(String const &Argument)
{
	size_t EqualsPosition;
	if ((EqualsPosition = Argument.find('=')) == String::npos)
		ProgramConfiguration[Argument] = {"", "command line"};
	else
	{
		assert(EqualsPosition < Argument.length());
		String Key = Argument.substr(0, EqualsPosition);
		String Value = Argument.substr(EqualsPosition + 1);
		ProgramConfiguration[Key] = {Value, "command line"};
	}
}

std::map<String, Configuration> const &GetProgramConfiguration() { return ProgramConfiguration; }

