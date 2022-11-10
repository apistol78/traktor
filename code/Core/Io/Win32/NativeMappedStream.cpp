/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/System.h"
#include "Core/Io/Win32/NativeMappedStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeMappedStream", NativeMappedStream, MemoryStream)

NativeMappedStream::NativeMappedStream(void* hFile, void* hFileMapping, void* ptr, int64_t size)
:	MemoryStream(ptr, size)
,	m_hFile(hFile)
,	m_hFileMapping(hFileMapping)
{
}

NativeMappedStream::~NativeMappedStream()
{
	T_EXCEPTION_GUARD_BEGIN

	close();

	T_EXCEPTION_GUARD_END
}

void NativeMappedStream::close()
{
	if (m_buffer)
	{
		UnmapViewOfFile(m_buffer);
		m_buffer = nullptr;
	}
	if (m_hFile)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
	}
	if (m_hFileMapping)
	{
		CloseHandle(m_hFileMapping);
		m_hFileMapping = NULL;
	}
}

}
