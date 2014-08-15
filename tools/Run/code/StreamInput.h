#ifndef StreamInput_H
#define StreamInput_H

#include <Core/Io/StringReader.h>
#include "IInput.h"

class StreamInput : public IInput
{
	T_RTTI_CLASS;

public:
	StreamInput(traktor::IStream* stream, traktor::IEncoding* encoding);

	virtual bool endOfFile();

	virtual std::wstring readChar();

	virtual std::wstring readLn();

private:
	traktor::Ref< traktor::IStream > m_stream;
	traktor::StringReader m_reader;
};

#endif	// StreamInput_H
