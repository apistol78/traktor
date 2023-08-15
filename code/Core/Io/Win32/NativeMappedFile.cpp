/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/System.h"
#include "Core/Io/Win32/NativeMappedFile.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeMappedFile", NativeMappedFile, IMappedFile)

NativeMappedFile::NativeMappedFile(void* hFile, void* hFileMapping, void* base, int64_t size)
:	m_hFile(hFile)
,	m_hFileMapping(hFileMapping)
,	m_base(base)
,	m_size(size)
{
}

NativeMappedFile::~NativeMappedFile()
{
	if (m_base)
	{
		UnmapViewOfFile(m_base);
		m_base = nullptr;
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

void* NativeMappedFile::getBase() const
{
	return m_base;
}

int64_t NativeMappedFile::getSize() const
{
	return m_size;
}

}
