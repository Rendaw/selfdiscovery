#ifndef SHARED_H
#define SHARED_H

#include <queue>

#include "ren-general/string.h"

struct InteractionError // When the system or user fails
{
	InteractionError(String const &Message);
	String Message;
};

struct ControllerError // When the controller misbehaves
{
	ControllerError(String const &Message);
	String Message;
};

String GetNextArgument(std::queue<String> &Arguments, String const &Name);

std::pair<bool, String> FindProgramArgument(String const &Name);

namespace Information
{
	// By using anchors, the following is guaranteed (more or less);
	// The item class is always instantiated before Respond is called.
	// The item class is only instantiated if Respond is called.
	// -- Note, Respond may be called multiple times for different pieces of information.  Initialization should gather all data that might be required multiple times.
	class Anchor
	{
		public:
			virtual ~Anchor(void);
			virtual void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out) = 0;
			virtual void Respond(std::queue<String> &&Arguments, std::ostream &Out) = 0;
	};

	template <typename ItemClass> class AnchorImplementation : public Anchor
	{
		public:
			AnchorImplementation(void) : AnchoredItem(nullptr) {}
			~AnchorImplementation(void) { delete AnchoredItem; }
			void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out) override
			{
				Out << "\tConfiguration for: " << ItemClass::GetIdentifier() << "\n";
				ItemClass::DisplayUserHelp(std::move(Arguments), Out); 
			}
			void Respond(std::queue<String> &&Arguments, std::ostream &Out) override
				{ (*this)->Respond(std::move(Arguments), Out); }
			ItemClass *operator->(void)
			{
				if (AnchoredItem == nullptr) AnchoredItem = new ItemClass;
				return AnchoredItem;
			}
		private:
			ItemClass *AnchoredItem;
	};
}

#endif // SHARED_H
