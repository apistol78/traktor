/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "Core/Io/Linux/NativeMappedFile.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeMappedFile", NativeMappedFile, IMappedFile)

NativeMappedFile::NativeMappedFile(int fd, void* base, int64_t size)
:	m_fd(fd)
,	m_base(base)
,	m_size(size)
{
}

NativeMappedFile::~NativeMappedFile()
{
	if (m_base)
	{
		munmap(m_base, m_size);
		m_base = nullptr;
	}
	if (m_fd >= 0)
	{
		close(m_fd);
		m_fd = -1;
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
