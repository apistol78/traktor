/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstdio>
#include "Core/Io/File.h"
#include "Core/Io/PNaCl/NativeStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeStream", NativeStream, IStream)

NativeStream::NativeStream(std::FILE* fp, uint32_t mode)
:	m_fp(fp)
,	m_mode(mode)
,	m_fileSize(0)
{
	if (m_mode == File::FmRead)
	{
		fseek(m_fp, 0, SEEK_END);
		m_fileSize = (int)ftell(m_fp);
		fseek(m_fp, 0, SEEK_SET);
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
	return (m_fp != 0 && m_mode == File::FmRead);
}

bool NativeStream::canWrite() const
{
	return (m_fp != 0 && (m_mode == File::FmWrite || m_mode == File::FmAppend));
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

	const int64_t fo[] = { SEEK_CUR, SEEK_END, SEEK_SET };
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
