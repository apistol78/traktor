/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/System/Win32/SharedMemoryWin32.h"

namespace traktor
{
	namespace
	{

#pragma pack(1)
struct Header
{
	LONG readerCount;
	LONG writerCount;
};
#pragma pack()

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryWin32", SharedMemoryWin32, ISharedMemory)

SharedMemoryWin32::SharedMemoryWin32()
:	m_hMap(NULL)
,	m_ptr(nullptr)
,	m_size(0)
{
}

SharedMemoryWin32::~SharedMemoryWin32()
{
	if (m_ptr)
	{
		UnmapViewOfFile(m_ptr);
		m_ptr = nullptr;
	}
	if (m_hMap)
	{
		CloseHandle(m_hMap);
		m_hMap = NULL;
	}
}

bool SharedMemoryWin32::create(const std::wstring& name, uint32_t size)
{
	bool initializeMemory = false;

	// Add space for header as well.
	size += sizeof(Header);

	m_hMap = OpenFileMapping(FILE_MAP_WRITE | FILE_MAP_READ, FALSE, name.c_str());
	if (m_hMap != NULL)
		T_DEBUG(L"Shared memory object \"" << name << L"\" (size " << size << L") opened");
	else
	{
		// No such mapping opened; create new mapping.
		m_hMap = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			size,
			name.c_str()
		);
		if (m_hMap == NULL)
			return false;

		T_DEBUG(L"Shared memory object \"" << name << L"\" (size " << size << L") created");

		// We've created the mapping; need to initialize header.
		initializeMemory = true;
	}

	// Map memory region.
	m_ptr = MapViewOfFile(m_hMap, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, size);
	if (!m_ptr)
	{
		CloseHandle(m_hMap); m_hMap = NULL;
		return false;
	}

	// Initialize header and memory region, ensure everything is zero.
	if (initializeMemory)
		std::memset(m_ptr, 0, size);

	m_size = size;
	return true;
}

const void* SharedMemoryWin32::acquireReadPointer(bool exclusive)
{
	Header* header = static_cast< Header* >(m_ptr);
	if (!header)
		return nullptr;

	// Wait until no writers so we can acquire a reader.
	for (;;)
	{
		while (InterlockedCompareExchange(&header->writerCount, 0, 0) != 0)
			Sleep(0);

		InterlockedIncrement(&header->readerCount);

		if (InterlockedCompareExchange(&header->writerCount, 0, 0) == 0)
			break;

		InterlockedDecrement(&header->readerCount);
	}

	return static_cast< void* >(header + 1);
}

void SharedMemoryWin32::releaseReadPointer()
{
	Header* header = static_cast< Header* >(m_ptr);
	if (!header)
		return;

	T_FATAL_ASSERT(header->readerCount >= 1);
	InterlockedDecrement(&header->readerCount);
}

void* SharedMemoryWin32::acquireWritePointer()
{
	Header* header = static_cast< Header* >(m_ptr);
	if (!header)
		return nullptr;

	// Wait until no writers.
	while (InterlockedCompareExchange(&header->writerCount, 1, 0) != 0)
		Sleep(0);

	// Wait until no readers.
	while (InterlockedCompareExchange(&header->readerCount, 0, 0) != 0)
		Sleep(0);

	return static_cast< void* >(header + 1);
}

void SharedMemoryWin32::releaseWritePointer()
{
	Header* header = static_cast< Header* >(m_ptr);
	if (!header)
		return;

	T_FATAL_ASSERT(header->writerCount == 1);
	InterlockedDecrement(&header->writerCount);
}

}
