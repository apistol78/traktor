#include "Core/System/OsX/SharedMemoryOsX.h"
#include "Core/Io/MemoryStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryOsX", SharedMemoryOsX, ISharedMemory)

SharedMemoryOsX::SharedMemoryOsX(uint32_t size)
:	m_data(new uint8_t [size])
,	m_size(size)
{
}
	
Ref< IStream > SharedMemoryOsX::read(bool exclusive)
{
	return new MemoryStream(m_data.ptr(), m_size, true, false);
}
	
Ref< IStream > SharedMemoryOsX::write()
{
	return new MemoryStream(m_data.ptr(), m_size, false, true);
}

}
