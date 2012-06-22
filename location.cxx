#include "location.h"

String InstallBinDirectory::GetIdentifier(void) { return "install-binary-directory"; }
void InstallBinDirectory::DisplayUserHelp(std::ostream &Out)
{
}

void InstallBinDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
}

String InstallDataDirectory::GetIdentifier(void);
void InstallDataDirectory::DisplayUserHelp(std::ostream &Out);
void InstallDataDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out);

String InstallGlobalConfigDirectory::GetIdentifier(void);
void InstallGlobalConfigDirectory::DisplayUserHelp(std::ostream &Out);
void InstallGlobalConfigDirectory::Respond(std::queue<String> &&Arguments, std::ostream &Out);

