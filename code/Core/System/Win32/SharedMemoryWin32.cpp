/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/MemoryStream.h"
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
	LONG dataSize;
};
#pragma pack()

class SharedReaderStream : public MemoryStream
{
public:
	SharedReaderStream(Header* header, void* buffer, uint32_t bufferSize)
	:	MemoryStream(buffer, bufferSize, true, false)
	,	m_header(header)
	{
	}

	virtual ~SharedReaderStream()
	{
		close();
	}

	virtual void close()
	{
		if (m_header)
		{
			InterlockedDecrement(&m_header->readerCount);
			MemoryStream::close();
			m_header = 0;
		}
	}

private:
	Header* m_header;
};

class SharedWriterStream : public MemoryStream
{
public:
	SharedWriterStream(Header* header, void* buffer, uint32_t bufferSize)
	:	MemoryStream(buffer, bufferSize, false, true)
	,	m_header(header)
	{
	}

	virtual ~SharedWriterStream()
	{
		close();
	}

	virtual void close()
	{
		if (m_header)
		{
			m_header->dataSize = tell();
			T_ASSERT (m_header->writerCount == 1);
			InterlockedDecrement(&m_header->writerCount);
			MemoryStream::close();
			m_header = 0;
		}
	}

private:
	Header* m_header;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryWin32", SharedMemoryWin32, ISharedMemory)

SharedMemoryWin32::SharedMemoryWin32()
:	m_hMap(NULL)
,	m_ptr(0)
,	m_size(0)
{
}

SharedMemoryWin32::~SharedMemoryWin32()
{
	if (m_ptr)
	{
		UnmapViewOfFile(m_ptr);
		m_ptr = 0;
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

	// Initialize header.
	if (initializeMemory)
	{
		Header* header = static_cast< Header* >(m_ptr);
		header->readerCount = 0;
		header->writerCount = 0;
		header->dataSize = 0;
	}

	m_size = size;
	return true;
}

Ref< IStream > SharedMemoryWin32::read(bool exclusive)
{
	Header* header = static_cast< Header* >(m_ptr);
	if (!header)
		return 0;

	for (;;)
	{
		while (InterlockedCompareExchange(&header->writerCount, 0, 0) != 0)
			Sleep(0);

		InterlockedIncrement(&header->readerCount);

		if (InterlockedCompareExchange(&header->writerCount, 0, 0) == 0)
			break;

		InterlockedDecrement(&header->readerCount);
	}

	return new SharedReaderStream(header, header + 1, header->dataSize);
}

Ref< IStream > SharedMemoryWin32::write()
{
	Header* header = static_cast< Header* >(m_ptr);
	if (!header)
		return 0;

	// Wait until no writers.
	while (InterlockedCompareExchange(&header->writerCount, 1, 0) != 0)
		Sleep(0);

	// Wait until no readers.
	while (InterlockedCompareExchange(&header->readerCount, 0, 0) != 0)
		Sleep(0);

	header->dataSize = 0;

	return new SharedWriterStream(header, header + 1, m_size);
}

bool SharedMemoryWin32::clear()
{
	Header* header = static_cast< Header* >(m_ptr);
	if (!header)
		return false;

	// Wait until no writers.
	while (InterlockedCompareExchange(&header->writerCount, 1, 0) != 0)
		Sleep(0);

	// Wait until no readers.
	while (InterlockedCompareExchange(&header->readerCount, 0, 0) != 0)
		Sleep(0);

	header->dataSize = 0;
	return true;
}

}
