#include "blank.h"

#include <iostream>

#include "ren-general/inputoutput.h"

String Blank::GetIdentifier(void) { return ""; }
void Blank::DisplayControllerHelp(OutputStream &Out) {}
void Blank::DisplayUserHelp(std::queue<String> &&Arguments, OutputStream &Out) {}
void Blank::Respond(std::queue<String> &&Arguments, OutputStream &Out)
	{ StandardErrorStream << "Warning: The controller made a blank request.  Ignoring..." << "\n" << OutputStream::Flush(); }

