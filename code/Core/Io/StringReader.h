/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Read strings from stream.
 * \ingroup Core
 */
class T_DLLCLASS StringReader : public Object
{
	T_RTTI_CLASS;

public:
	StringReader(IStream* stream, IEncoding* encoding);

	/*! Read character from stream. */
	wchar_t readChar();

	/*! Read string from stream until end-of-line or end-of-file. */
	int64_t readLine(std::wstring& out);

private:
	Ref< IStream > m_stream;
	Ref< IEncoding > m_encoding;
	uint8_t m_buffer[IEncoding::MaxEncodingSize];
	int64_t m_count;
};

}

