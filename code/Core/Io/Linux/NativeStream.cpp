/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstdio>
#include <sys/stat.h>
#include "Core/Io/File.h"
#include "Core/Io/Linux/NativeStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeStream", NativeStream, IStream)

NativeStream::NativeStream(std::FILE* fp, uint32_t mode)
:	m_fp(fp)
,	m_mode(mode)
,	m_fileSize(0)
{
	if ((m_mode & File::FmRead) != 0)
	{
		struct stat st;
		fstat(fileno(m_fp), &st);
		m_fileSize = st.st_size;
	}
}

NativeStream::~NativeStream()
{
	close();
}

void NativeStream::close()
{
	if (m_fp)
	{
		flush();
		fclose(m_fp);
		m_fp = NULL;
	}
}

bool NativeStream::canRead() const
{
	return (m_fp != 0 && ((m_mode & File::FmRead) != 0));
}

bool NativeStream::canWrite() const
{
	return (m_fp != 0 && ((m_mode & (File::FmWrite | File::FmAppend)) != 0));
}

bool NativeStream::canSeek() const
{
	return (m_fp != 0);
}

int64_t NativeStream::tell() const
{
	return (m_fp != 0) ? (int64_t)ftell(m_fp) : 0;
}

int64_t NativeStream::available() const
{
	return (m_fp != 0) ? ((int64_t)m_fileSize - tell()) : 0;
}

int64_t NativeStream::seek(SeekOriginType origin, int64_t offset)
{
	if (m_fp == 0)
		return 0;

	const int fo[] = { SEEK_CUR, SEEK_END, SEEK_SET };
	return (int64_t)fseek(m_fp, offset, fo[origin]);
}

int64_t NativeStream::read(void* block, int64_t nbytes)
{
	if (m_fp == 0)
		return 0;

	return int64_t(fread(block, 1, nbytes, m_fp));
}

int64_t NativeStream::write(const void* block, int64_t nbytes)
{
	if (m_fp == 0)
		return 0;

	return int64_t(fwrite(block, 1, nbytes, m_fp));
}

void NativeStream::flush()
{
	if (m_fp != 0)
		fflush(m_fp);
}

}
