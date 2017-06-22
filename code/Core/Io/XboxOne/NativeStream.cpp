/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/XboxOne/NativeStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeStream", NativeStream, IStream)

NativeStream::NativeStream()
{
}

NativeStream::~NativeStream()
{
	T_EXCEPTION_GUARD_BEGIN
	
	close();

	T_EXCEPTION_GUARD_END
}

void NativeStream::close()
{
}

bool NativeStream::canRead() const
{
	return false;
}

bool NativeStream::canWrite() const
{
	return false;
}

bool NativeStream::canSeek() const
{
	return false;
}

int64_t NativeStream::tell() const
{
	return 0;
}

int64_t NativeStream::available() const
{
	return 0;
}

int64_t NativeStream::seek(SeekOriginType origin, int64_t offset)
{
	return 0;
}

int64_t NativeStream::read(void* block, int64_t nbytes)
{
	return 0;
}

int64_t NativeStream::write(const void* block, int64_t nbytes)	
{
	return 0;
}

void NativeStream::flush()
{
}

}
