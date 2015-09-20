#ifndef traktor_run_StreamInput_H
#define traktor_run_StreamInput_H

#include "Core/Io/StringReader.h"
#include "Run/App/IInput.h"

namespace traktor
{
	namespace run
	{

class StreamInput : public IInput
{
	T_RTTI_CLASS;

public:
	StreamInput(IStream* stream, IEncoding* encoding);

	virtual bool endOfFile();

	virtual std::wstring readChar();

	virtual std::wstring readLn();

private:
	Ref< IStream > m_stream;
	StringReader m_reader;
};

	}
}

#endif	// traktor_run_StreamInput_H
