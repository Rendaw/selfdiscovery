#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "ren-general/string.h"
#include "ren-general/filesystem.h"

struct Configuration
{
	String Value;
	String Source;
};

std::pair<bool, String> FindConfiguration(String const &Name);

void LoadConfigurationFile(FilePath const &File);
void LoadConfigurationCommandline(String const &Argument);

std::map<String, Configuration> const &GetProgramConfiguration();

#endif

