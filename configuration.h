#ifndef CONFIGURATION_H
#define CONFIGURATION_H

struct Configuration
{
	String Value;
	String Source;
};

std::pair<bool, String> FindConfiguration(String const &Name);

void LoadConfigurationFile(FilePath const &File);
void LoadConfiruationCommandline(String const &Argument);

#endif

