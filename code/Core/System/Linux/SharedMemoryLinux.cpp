#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "Core/Log/Log.h"
#include "Core/Misc/MD5.h"
#include "Core/Misc/TString.h"
#include "Core/System/Linux/SharedMemoryLinux.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryLinux", SharedMemoryLinux, ISharedMemory)

SharedMemoryLinux::SharedMemoryLinux(const std::wstring& name, uint32_t size)
:	m_name(name)
,	m_fd(0)
,	m_ptr(nullptr)
,	m_size(size)
{
	// Create a digest of name, cannot use qualified paths on posix.
	MD5 md5;
	md5.createFromString(name);
	m_name = std::wstring(L"/") + md5.format();

	// Create shared memory object.
	m_fd = shm_open(wstombs(m_name).c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (ftruncate(m_fd, m_size) != -1)
	{
		m_ptr = mmap(nullptr, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
		if (!m_ptr)
			log::error << L"SharedMemoryLinux failed; mmap of \"" << m_name << L"\" failed." << Endl;
	}
	else
		log::error << L"SharedMemoryLinux failed; ftruncate of \"" << m_name << L"\" failed." << Endl;
}

SharedMemoryLinux::~SharedMemoryLinux()
{
	if (m_ptr)
	{
		munmap(m_ptr, m_size);
		shm_unlink(wstombs(m_name).c_str());
	}
}

const void* SharedMemoryLinux::acquireReadPointer(bool exclusive)
{
	return m_ptr;
}

void SharedMemoryLinux::releaseReadPointer()
{
}

void* SharedMemoryLinux::acquireWritePointer()
{
	return m_ptr;
}

void SharedMemoryLinux::releaseWritePointer()
{
}

}
