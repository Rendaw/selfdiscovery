#include "shared.h"

#include <unistd.h>
#include <iostream>
#include <cstring>
#ifdef _WIN32
#include <fcntl.h>
#else
#include <csignal>
#include <sys/wait.h>
#endif

#include "ren-general/arrangement.h"

InteractionError::InteractionError(String const &Explanation) : Explanation(Explanation) {}

StringSplitter::StringSplitter(Set<char> const &Delimiters, bool DropBlanks) : 
	Delimiters(Delimiters), DropBlanks(DropBlanks), HotSlash(false), HotQuote(false) {}

StringSplitter &StringSplitter::Process(String const &Input)
{
	String Buffer;
	Buffer.reserve(1000);

	char const Slash = '\\';
	char const Quote = '"';
	for (unsigned int CharacterIndex = 0; CharacterIndex < Input.length(); ++CharacterIndex)
	{
		char const &CurrentCharacter = Input[CharacterIndex];
		if (!HotSlash)
		{
			if (CurrentCharacter == Slash)
			{
				HotSlash = true;
				continue;
			}
		
			if (CurrentCharacter == Quote)
			{
				HotQuote = !HotQuote;
				continue;
			}
		}

		if (HotSlash)
		{
			HotSlash = false;
		}

		if (!HotQuote && Delimiters.Contains(CurrentCharacter))
		{
			if (!DropBlanks || !Buffer.empty())
				Out.push(Buffer);
			Buffer.clear();
			continue;
		}
		
		Buffer.push_back(CurrentCharacter);
	}

	if (!DropBlanks || !Buffer.empty())
		Out.push(Buffer);

	return *this;
}

bool StringSplitter::Finished(void) { return !HotSlash && !HotQuote; }

std::queue<String> &StringSplitter::Results(void) { return Out; }

