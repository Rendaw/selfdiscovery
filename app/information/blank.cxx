#include "blank.h"

#include <iostream>

String Blank::GetIdentifier(void) { return ""; }
void Blank::DisplayControllerHelp(std::ostream &Out) {}
void Blank::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out) {}
void Blank::Respond(std::queue<String> &&Arguments, std::ostream &Out)
	{ std::cerr << "Warning: The controller made a blank request.  Ignoring..." << std::endl; }

