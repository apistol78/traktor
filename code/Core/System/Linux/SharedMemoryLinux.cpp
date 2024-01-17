/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "Core/Misc/MD5.h"
#include "Core/Misc/TString.h"
#include "Core/System/Linux/SharedMemoryLinux.h"
#include "Core/Thread/Atomic.h"

namespace traktor
{
	namespace
	{

#pragma pack(1)
struct Header
{
	int32_t readerCount;
	int32_t writerCount;
};
#pragma pack()

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryLinux", SharedMemoryLinux, ISharedMemory)

SharedMemoryLinux::SharedMemoryLinux()
:	m_fd(0)
,	m_ptr(nullptr)
,	m_size(0)
{
}

SharedMemoryLinux::~SharedMemoryLinux()
{
	if (m_ptr)
		munmap(m_ptr, m_size);
	if (m_fd)
		close(m_fd);
}

bool SharedMemoryLinux::create(const std::wstring& name, uint32_t size)
{
	bool initializeMemory = false;

	// Add space for header as well.
	size += sizeof(Header);

	// Create a digest of name, cannot use qualified paths on posix.
	MD5 md5;
	md5.createFromString(name);
	m_name = std::wstring(L"/") + md5.format();

	// Open shared memory object.
	m_fd = shm_open(wstombs(m_name).c_str(), O_RDWR, S_IRUSR | S_IWUSR);
	if (m_fd < 0)
	{
		// Shared memory object doesn't exist, create new.
		m_fd = shm_open(wstombs(m_name).c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
		if (m_fd < 0)
			return false;

		// Set size of shared object.
		if (ftruncate(m_fd, size) == -1)
		{
			close(m_fd);
			m_fd = 0;
			return false;
		}

		initializeMemory = true;
	}

	// Map into virtual memory.
	m_ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
	if (!m_ptr)
	{
		close(m_fd);
		m_fd = 0;
		return false;
	}

	// Initialize memory if necessary.
	if (initializeMemory)
		std::memset(m_ptr, 0, size);

	m_size = size;
	return true;
}

const void* SharedMemoryLinux::acquireReadPointer(bool exclusive)
{
	Header* header = static_cast< Header* >(m_ptr);
	if (!header)
		return nullptr;

	// Wait until no writers so we can acquire a reader.
	for (;;)
	{
		while (Atomic::compareAndSwap(header->writerCount, 0, 0) != 0)
			sched_yield();

		Atomic::increment(header->readerCount);

		if (Atomic::compareAndSwap(header->writerCount, 0, 0) == 0)
			break;

		Atomic::decrement(header->readerCount);
	}

	return static_cast< void* >(header + 1);
}

void SharedMemoryLinux::releaseReadPointer()
{
	Header* header = static_cast< Header* >(m_ptr);
	if (!header)
		return;

	T_FATAL_ASSERT(header->readerCount >= 1);
	Atomic::decrement(header->readerCount);
}

void* SharedMemoryLinux::acquireWritePointer()
{
	Header* header = static_cast< Header* >(m_ptr);
	if (!header)
		return nullptr;

	// Wait until no writers.
	while (Atomic::compareAndSwap(header->writerCount, 0, 1) != 0)
		sched_yield();

	// Wait until no readers.
	while (Atomic::compareAndSwap(header->readerCount, 0, 0) != 0)
		sched_yield();

	return static_cast< void* >(header + 1);
}

void SharedMemoryLinux::releaseWritePointer()
{
	Header* header = static_cast< Header* >(m_ptr);
	if (!header)
		return;

	T_FATAL_ASSERT(header->writerCount == 1);
	Atomic::decrement(header->writerCount);
}

}
