#ifndef traktor_run_StreamOutput_H
#define traktor_run_StreamOutput_H

#include "Core/Io/FileOutputStream.h"
#include "Run/App/IOutput.h"

namespace traktor
{
	namespace run
	{

class StreamOutput : public IOutput
{
	T_RTTI_CLASS;

public:
	StreamOutput(IStream* stream, IEncoding* encoding);

	StreamOutput(IStream* stream, IEncoding* encoding, const std::wstring& lineEnding);

	virtual void print(const std::wstring& s);

	virtual void printLn(const std::wstring& s);

private:
	FileOutputStream m_output;
};

	}
}

#endif	// traktor_run_StreamOutput_H
