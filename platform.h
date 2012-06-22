#ifndef PLATFORM_H
#define PLATFORM_H

class Platform
{
	public:
		Platform(void);
		static String GetIdentifier(void);
		static void DisplayUserHelp(std::ostream &Out);
		void Respond(std::queue<String> &&Arguments, std::ostream &Out);
		String GetFamily(void);
		String GetMember(void);

	private:
		String Family, Member;
};

#endif // PLATFORM_H

