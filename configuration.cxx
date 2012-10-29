#include "configuration.h"

std::map<String, Configuration> ProgramConfiguration;

std::pair<bool, String> FindConfiguration(String const &Name)
{
	for (Set<String>::iterator ArgumentFound = ProgramArguments.lower_bound(Name); ArgumentFound != ProgramArguments.end(); ArgumentFound++)
	{
		if (ArgumentFound->length() < Name.length()) break;
		if (ArgumentFound->substr(0, Name.length()) != Name) break;
		if (ArgumentFound->length() == Name.length()) 
			return std::pair<bool, String>(true, String());
		if ((*ArgumentFound)[Name.length()] != '=') continue;
		return std::pair<bool, String>(true, ArgumentFound->substr(Name.length() + 1, String::npos));
	}
	return std::pair<bool, String>(false, String());
}

void LoadConfigArguments(FilePath const &File)
{
	try 
	{
		FileInput UserOverrides = File;
	}
	catch (Error::System &Error) { return; }

	String ConfigArgument;
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
		assert(RangeD(1,2).Contains(SplitKeyValue.Results()));
		String Key = Results().front();
		Results().pop();
		String Value = Results.back();
		ProgramConfiguration[Key] = {Value, File};
	}
}

void LoadConfiruationCommandline(String const &Argument)
{
	size_t EqualsPosition;
	if ((EqualsPosition = Argument.find('=')) == String::npos)
		ProgramConfiguration[Key] = {"", "command line"};
	else
	{
		assert(EqualsPosition < Argument.length());
		String Key = Argument.substr(0, EqualsPosition);
		String Value = Argument.substr(EqualsPosition + 1);
		ProgramConfiguration[Key] = {Value, "command line"};
	}
}

