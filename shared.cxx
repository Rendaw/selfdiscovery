#include "shared.h"

InteractionError::InteractionError(String const &Message) : Message(Message) {}
ControllerError::ControllerError(String const &Message) : Message(Message) {}

namespace Information
{
	Anchor::~Anchor(void) {}
}
