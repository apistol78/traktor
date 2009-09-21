#include "Core/System/OsX/SharedMemoryOsX.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Heap/GcNew.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryOsX", SharedMemoryOsX, SharedMemory)

SharedMemoryOsX::SharedMemoryOsX(uint32_t size)
:	m_data(new uint8_t [size])
,	m_size(size)
{
}
	
Stream* SharedMemoryOsX::read(bool exclusive)
{
	return gc_new< MemoryStream >(m_data.ptr(), m_size, true, false);
}
	
Stream* SharedMemoryOsX::write()
{
	return gc_new< MemoryStream >(m_data.ptr(), m_size, false, true);
}

}
