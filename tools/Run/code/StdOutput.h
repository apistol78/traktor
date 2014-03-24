#ifndef StdOutput_H
#define StdOutput_H

#include <cstdio>
#include "IOutput.h"

class StdOutput : public IOutput
{
	T_RTTI_CLASS;

public:
	StdOutput(FILE* handle);

	virtual void print(const std::wstring& s);

	virtual void printLn(const std::wstring& s);

private:
	FILE* m_handle;
};

#endif	// StdOutput_H
