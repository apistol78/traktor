/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileOutputStreamBuffer.h"
#include "Core/Io/IEncoding.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{

FileOutputStreamBuffer::FileOutputStreamBuffer(IStream* stream, IEncoding* encoding)
:	m_stream(stream)
,	m_encoding(encoding)
,	m_encodedSize(0)
{
}

void FileOutputStreamBuffer::close()
{
	safeClose(m_stream);
}

int32_t FileOutputStreamBuffer::overflow(const wchar_t* buffer, int32_t count)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT(count > 0);

	uint32_t maxEncodedSize = count * IEncoding::MaxEncodingSize;
	if (maxEncodedSize > m_encodedSize)
	{
		m_encodedSize = maxEncodedSize;
		m_encoded.reset(new uint8_t [m_encodedSize]);
	}

	int32_t encodedCount = m_encoding->translate(buffer, count, m_encoded.ptr());
	if (encodedCount < 0)
		return -1;

	if (m_stream->write(m_encoded.c_ptr(), encodedCount) != encodedCount)
		return -1;

	return count;
}

}
