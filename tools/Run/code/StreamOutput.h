#ifndef StreamOutput_H
#define StreamOutput_H

#include <Core/Io/FileOutputStream.h>
#include "IOutput.h"

class StreamOutput : public IOutput
{
	T_RTTI_CLASS;

public:
	StreamOutput(traktor::IStream* stream, traktor::IEncoding* encoding);

	StreamOutput(traktor::IStream* stream, traktor::IEncoding* encoding, const std::wstring& lineEnding);

	virtual void print(const std::wstring& s);

	virtual void printLn(const std::wstring& s);

private:
	traktor::FileOutputStream m_output;
};

#endif	// StreamOutput_H
