/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "Core/Io/MemoryStream.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/System/OsX/SharedMemoryOsX.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryOsX", SharedMemoryOsX, ISharedMemory)

SharedMemoryOsX::SharedMemoryOsX(uint32_t size)
:	m_size(size)
{
	int32_t handle = open("/tmp/traktor.shm", O_RDWR);
	if (handle == -1)
	{
		handle = open("/tmp/traktor.shm", O_CREAT | O_RDWR, 0666);
		if (handle != -1)
		{
			AutoArrayPtr< uint8_t > dummy(new uint8_t [1024]);
			for (uint32_t i = 0; i < (size + 1023) / 1024; ++i)
				::write(handle, dummy.ptr(), 1024);

			close(handle);

			handle = open("/tmp/traktor.shm", O_RDWR);
		}
	}

	if (handle != -1)
	{
		m_buffer = mmap(0, m_size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, handle, 0);
		close(handle);
	}
}

SharedMemoryOsX::~SharedMemoryOsX()
{
	if (m_buffer)
	{
		munmap(m_buffer, m_size);
		m_buffer = 0;
	}
}
	
Ref< IStream > SharedMemoryOsX::read(bool exclusive)
{
	return new MemoryStream(m_buffer, m_size, true, false);
}
	
Ref< IStream > SharedMemoryOsX::write()
{
	return new MemoryStream(m_buffer, m_size, false, true);
}

bool SharedMemoryOsX::clear()
{
	std::memset(m_buffer, 0, m_size);
	return true;
}

}
