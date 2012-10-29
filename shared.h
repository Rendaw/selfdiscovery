#ifndef SHARED_H
#define SHARED_H

#include <queue>
#include <vector>
#include <memory>

#include "ren-general/string.h"
#include "ren-general/arrangement.h"
#include "ren-general/filesystem.h"

struct InteractionError // When the system or user fails
{
	InteractionError(String const &Message);
	String Message;
};

class StringSplitter
{
	public:
		StringSplitter(Set<char> const &Delimiters, bool DropBlanks);
		StringSplitter &Process(String const &Input);
		bool Unfinished(void); // Open quotes or unused escape (at end of input)
		std::queue<String> &Results(void);
	private:
		Set<char> const &Delimiters;
		bool DropBlanks;

		bool HotSlash;
		bool HotQuote;
		std::queue<String> Out;
};

#endif // SHARED_H
