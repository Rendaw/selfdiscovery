#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#include <vector>
#include <memory>

#include "ren-general/string.h"
#include "ren-general/filesystem.h"

class SubprocessOutStream
{
	public:
		SubprocessOutStream(void);
		~SubprocessOutStream(void);
		void Associate(int FileDescriptor);
		String ReadLine(void);
		bool HasFailed(void);
		void ReadToEnd(void);

	private:
		int FileDescriptor;
		bool Failed;
};

class SubprocessInStream
{
	public:
		SubprocessInStream(void);
		~SubprocessInStream(void);
		void Associate(int FileDescriptor);
		void Write(String const &Contents = String("\n"));
	private:
		int FileDescriptor;
};

class Subprocess
{
	public:
		Subprocess(FilePath const &Execute, std::vector<String> const &Arguments);
		SubprocessOutStream Out;
		SubprocessInStream In;
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
