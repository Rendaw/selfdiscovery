#include "ren-general/string.h"

struct InteractionError // When the system or user fails
{
	InteractionError(String const &Message) : Message(Message) {}
	String Message;
};

struct ControllerError // When the controller misbehaves
{
	ControllerError(String const &Message) : Message(Message) {}
	String Message;
};

namespace Information
{
	class Anchor
	{
		public:
			virtual ~Anchor(void) {}
			virtual void DisplayUserHelp(std::ostream &Out) = 0;
			virtual void Respond(std::queue<String> &&Arguments, std::ostream &Out) = 0;
	};

	template <typename ItemClass> class AnchorImplementation : public Anchor
	{
		public:
			AnchorImplementation(void) : AnchoredItem(nullptr) {}
			~AnchorImplementation(void) { delete AnchoredItem; }
			void DisplayUserHelp(std::ostream &Out) { ItemClass::DisplayUserHelp(Out); }
			void Respond(std::queue<String> &&Arguments, std::ostream &Out)
			{ 
				if (AnchoredItem == nullptr) AnchoredItem = new ItemClass;
				AnchoredItem->Respond(std::move(Arguments), Out);
			}
		private:
			ItemClass *AnchoredItem;
	};
}

