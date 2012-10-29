#ifndef INFORMATION_H
#define INFORMATION_H

String GetArgument(Script &State, String const &Name); // Throws Error::User if missing or empty
String GetOptionalArgument(Script &State, String const &Name); // Returns empty string if missing
bool GetFlag(Script &State, String const &Name);

class HelpItemCollector : public std::map<String, Set<String> >
{
	public:
		Add(String const &Argument, String const &NewDescription);
};

namespace Information
{
	// By using anchors, the following is guaranteed (more or less);
	// The item class is always instantiated before Respond is called or another information item needs it.
	// The item class is only instantiated if Respond is called or if another information item needs it.
	// -- Note, Respond may be called multiple times for different pieces of information.  Initialization should gather and cache all data that might be required multiple times.
	class Anchor
	{
		public:
			virtual ~Anchor(void);
			virtual void DisplayControllerHelp(void) = 0;
			virtual Script::Function GetUserHelpCallback(Script &State, std::map<String, String> &HelpItems) = 0;
			virtual Script::Function GetCallback(void) = 0;
	};

	template <typename ItemClass> class AnchorImplementation : public Anchor
	{
		public:
			AnchorImplementation(void) : AnchoredItem(nullptr) {}
			~AnchorImplementation(void) { delete AnchoredItem; }
			
			void DisplayControllerHelp(void) override
				{ ItemClass::DisplayControllerHelp(); }
			
			Script::Function GetUserHelpCallback(Script &State, std::map<String, String> &HelpItems) override
			{
				return [&](Script State) -> int
				{
					ItemClass::DisplayUserHelp(State, HelpItems);
					return 0;
				};
			}

			Script::Function GetCallback(void) override
			{
				return [&](Script State) -> int
				{
					if (AnchoredItem == nullptr) AnchoredItem = new ItemClass;
					try 
					{
						State.PushTable();
						AnchoredItem->Respond(State);
						if (State.IsEmpty())
						{
							State.Pop();
							return 0;
						}
						return 1;
					}
					catch (Error::User &Failure)
					{
						StandardErrorStream << "Controller error - please contact the controller's maintainer with this information: " << Failure.Explanation << "\n" << OutputStream::Flush();
						throw Failure;
					}
					catch (Error::System &Failure)
					{
						StandardErrorStream << "Internal error - please contact SelfDiscovery's maintainer with this information: " << Failure.Explanation << "\n" << OutputStream::Flush();
						throw Error::System("Internal error: " + Failure.Explanation);
					}
					catch (InteractionError &Failure)
					{
						StandardErrorStream << Failure.Message << "\n" << OutputStream::Flush();
						throw Error::System("Information gathering failed: " + Failure.Message);
					}
				};
			}
			
			ItemClass *operator->(void)
			{
				if (AnchoredItem == nullptr) AnchoredItem = new ItemClass;
				return AnchoredItem;
			}
		private:
			ItemClass *AnchoredItem;
	};
}

#endif

