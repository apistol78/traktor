#ifndef traktor_run_StdOutput_H
#define traktor_run_StdOutput_H

#include <cstdio>
#include "Run/App/IOutput.h"

namespace traktor
{
	namespace run
	{

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

	}
}

#endif	// traktor_run_StdOutput_H
