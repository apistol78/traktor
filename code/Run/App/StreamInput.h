#pragma once

#include "Core/Io/StringReader.h"
#include "Run/App/IInput.h"

namespace traktor
{
	namespace run
	{

/*! \brief Stream input reader.
 * \ingroup Run
 */
class StreamInput : public IInput
{
	T_RTTI_CLASS;

public:
	StreamInput(IStream* stream, IEncoding* encoding);

	virtual bool endOfFile() override final;

	virtual std::wstring readChar() override final;

	virtual std::wstring readLn() override final;

private:
	Ref< IStream > m_stream;
	StringReader m_reader;
};

	}
}

