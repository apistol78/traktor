#ifndef traktor_StringReader_H
#define traktor_StringReader_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Io/Encoding.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Stream;

/*! \brief Read strings from stream.
 * \ingroup Core
 */
class T_DLLCLASS StringReader : public Object
{
	T_RTTI_CLASS(StringReader)

public:
	StringReader(Stream* stream, Encoding* encoding);

	/*! \brief Read string from stream until end-of-line or end-of-file. */
	int readLine(std::wstring& out);

private:
	Ref< Stream > m_stream;
	Ref< Encoding > m_encoding;
	uint8_t m_buffer[Encoding::MaxEncodingSize];
	int m_count;
};

}

#endif	// traktor_StringReader_H
