#include "shared.h"

#include "ren-general/arrangement.h"

InteractionError::InteractionError(String const &Message) : Message(Message) {}
ControllerError::ControllerError(String const &Message) : Message(Message) {}

std::queue<String> SplitString(String const &Input, Set<char> const &Delimiters, bool DropBlanks)
{
	std::queue<String> Out;

	char const Slash = '\\';
	char const Quote = '"';
	bool HotSlash = false, HotQuote = false;
	unsigned int WordStart = 0;
	for (unsigned int CharacterIndex = 0; CharacterIndex < Input.length(); ++CharacterIndex)
	{
		char const &CurrentCharacter = Input[CharacterIndex];
		if (CurrentCharacter == Slash)
		{
			HotSlash = !HotSlash;
			continue;
		}

		if (HotSlash)
		{
			HotSlash = false;
			continue;
		}

		if (CurrentCharacter == Quote)
		{
			HotQuote = !HotQuote;
			continue;
		}

		if (!HotQuote && Delimiters.Contains(CurrentCharacter))
		{
			if (!DropBlanks || (CharacterIndex - WordStart > 1))
				Out.push(Input.substr(WordStart, CharacterIndex));
			WordStart = CharacterIndex + 1;
		}
	}

	if (!DropBlanks || (Input.length() - WordStart > 1))
		Out.push(Input.substr(WordStart, Input.length()));

	return std::move(Out);
}

String GetNextArgument(std::queue<String> &Arguments, String const &Name)
{
	if (Arguments.empty())
		throw ControllerError("Request ended before providing " + Name);
	String NextArgument = Arguments.front();
	Arguments.pop();
	return std::move(NextArgument);
}

extern Set<String> ProgramArguments;

std::pair<bool, String> FindProgramArgument(String const &Name)
{
	bool Found = false;
	Set<String>::iterator ArgumentFound = ProgramArguments.find(Name);
	String Value;
	if (ArgumentFound != ProgramArguments.end())
		Found = true;
	else if ((ArgumentFound = ProgramArguments.lower_bound(Name)) != ProgramArguments.end())
	{
		size_t EqualFound = ArgumentFound->find('=');
		String Front = *ArgumentFound;
		if (EqualFound != String::npos)
			Front = ArgumentFound->substr(0, EqualFound);
		if (Front == Name)
		{
			Found = true;
			Value = ArgumentFound->substr(EqualFound + 1, String::npos);
		}
	}

	return std::pair<bool, String>(Found, Value);
}

namespace Information
{
	Anchor::~Anchor(void) {}
}
