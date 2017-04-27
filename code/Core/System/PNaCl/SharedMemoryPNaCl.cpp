/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Io/MemoryStream.h"
#include "Core/System/PNaCl/SharedMemoryPNaCl.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryPNaCl", SharedMemoryPNaCl, ISharedMemory)

SharedMemoryPNaCl::SharedMemoryPNaCl(uint32_t size)
:	m_data(new uint8_t [size])
,	m_size(size)
{
}

Ref< IStream > SharedMemoryPNaCl::read(bool exclusive)
{
	return new MemoryStream(m_data.ptr(), m_size, true, false);
}

Ref< IStream > SharedMemoryPNaCl::write()
{
	return new MemoryStream(m_data.ptr(), m_size, false, true);
}

bool SharedMemoryPNaCl::clear()
{
	std::memset(m_data.ptr(), m_size, 0);
	return true;
}

}
