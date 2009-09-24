#include "Core/System/Linux/SharedMemoryLinux.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Heap/GcNew.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryLinux", SharedMemoryLinux, SharedMemory)

SharedMemoryLinux::SharedMemoryLinux(uint32_t size)
:	m_data(new uint8_t [size])
,	m_size(size)
{
}
	
Stream* SharedMemoryLinux::read(bool exclusive)
{
	return gc_new< MemoryStream >(m_data.ptr(), m_size, true, false);
}
	
Stream* SharedMemoryLinux::write()
{
	return gc_new< MemoryStream >(m_data.ptr(), m_size, false, true);
}

}
