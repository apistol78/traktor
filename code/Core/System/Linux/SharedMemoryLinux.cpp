#include <cstring>
#include "Core/Io/MemoryStream.h"
#include "Core/System/Linux/SharedMemoryLinux.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryLinux", SharedMemoryLinux, ISharedMemory)

SharedMemoryLinux::SharedMemoryLinux(uint32_t size)
:	m_data(new uint8_t [size])
,	m_size(size)
{
}

Ref< IStream > SharedMemoryLinux::read(bool exclusive)
{
	return new MemoryStream(m_data.ptr(), m_size, true, false);
}

Ref< IStream > SharedMemoryLinux::write()
{
	return new MemoryStream(m_data.ptr(), m_size, false, true);
}

bool SharedMemoryLinux::clear()
{
	std::memset(m_data.ptr(), 0, m_size);
	return true;
}

}
