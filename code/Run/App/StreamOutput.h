#pragma once

#include "Core/Io/FileOutputStream.h"
#include "Run/App/IOutput.h"

namespace traktor
{
	namespace run
	{

/*! Stream output writer.
 * \ingroup Run
 */
class StreamOutput : public IOutput
{
	T_RTTI_CLASS;

public:
	StreamOutput(IStream* stream, IEncoding* encoding);

	StreamOutput(IStream* stream, IEncoding* encoding, const std::wstring& lineEnding);

	virtual void print(const std::wstring& s) override final;

	virtual void printLn(const std::wstring& s) override final;

private:
	FileOutputStream m_output;
};

	}
}

