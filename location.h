#ifndef LOCATION_H
#define LOCATION_H

class LocationBinDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayUserHelp(std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
};

class LocationDataDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayUserHelp(std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
};

class LocationGlobalConfigDirectory
{
	public:
		static String GetIdentifier(void);
		static void DisplayUserHelp(std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
};

#endif // LOCATION_H

