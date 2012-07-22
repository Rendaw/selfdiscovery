#ifndef SHARED_H
#define SHARED_H

#include <queue>
#include <vector>
#include <memory>

#include "ren-general/string.h"
#include "ren-general/arrangement.h"
#include "ren-general/filesystem.h"

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

std::queue<String> SplitString(String const &Input, Set<char> const &Delimiters, bool DropBlanks);

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
			virtual void DisplayControllerHelp(std::ostream &Out) = 0;
			virtual void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out) = 0;
			virtual void Respond(std::queue<String> &&Arguments, std::ostream &Out) = 0;
	};

	template <typename ItemClass> class AnchorImplementation : public Anchor
	{
		public:
			AnchorImplementation(void) : AnchoredItem(nullptr) {}
			~AnchorImplementation(void) { delete AnchoredItem; }
			
			void DisplayControllerHelp(std::ostream &Out) override
				{ ItemClass::DisplayControllerHelp(Out); }
			
			void DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out) override
			{
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

class SubprocessInStream
{
	public:
		SubprocessInStream(void);
		~SubprocessInStream(void);
		void Associate(int FileDescriptor);
		String ReadLine(void);
		bool HasFailed(void);
		void ReadToEnd(void);

	private:
		int FileDescriptor;
		bool Failed;
};

class SubprocessOutStream
{
	public:
		SubprocessOutStream(void);
		~SubprocessOutStream(void);
		void Associate(int FileDescriptor);
		void Write(String const &Contents = String("\n"));
	private:
		int FileDescriptor;
};

class Subprocess
{
	public:
		Subprocess(FilePath const &Execute, std::vector<String> const &Arguments);
		SubprocessInStream In;
		SubprocessOutStream Out;
		void Kill(void);
		int GetResult(void);
	private:
#ifdef _WIN32
		PROCESS_INFORMATION ChildStatus;
#else
		pid_t ChildID;
#endif
		bool ResultRetrieved;
		int Result;
};

#endif // SHARED_H
