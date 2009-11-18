#ifndef traktor_StringReader_H
#define traktor_StringReader_H

#include "Core/Object.h"
#include "Core/Io/IEncoding.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

/*! \brief Read strings from stream.
 * \ingroup Core
 */
class T_DLLCLASS StringReader : public Object
{
	T_RTTI_CLASS;

public:
	StringReader(IStream* stream, IEncoding* encoding);

	/*! \brief Read string from stream until end-of-line or end-of-file. */
	int readLine(std::wstring& out);

private:
	Ref< IStream > m_stream;
	Ref< IEncoding > m_encoding;
	uint8_t m_buffer[IEncoding::MaxEncodingSize];
	int m_count;
};

}

#endif	// traktor_StringReader_H
