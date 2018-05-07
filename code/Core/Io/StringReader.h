/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_StringReader_H
#define traktor_StringReader_H

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
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

	/*! \brief Read character from stream. */
	wchar_t readChar();

	/*! \brief Read string from stream until end-of-line or end-of-file. */
	int64_t readLine(std::wstring& out);

private:
	Ref< IStream > m_stream;
	Ref< IEncoding > m_encoding;
	uint8_t m_buffer[IEncoding::MaxEncodingSize];
	int64_t m_count;
};

}

#endif	// traktor_StringReader_H
