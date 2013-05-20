#include "Core/Io/MemoryStream.h"
#include "Core/System/Emscripten/SharedMemoryEmscripten.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryEmscripten", SharedMemoryEmscripten, ISharedMemory)

SharedMemoryEmscripten::SharedMemoryEmscripten(uint32_t size)
:	m_data(new uint8_t [size])
,	m_size(size)
{
}

Ref< IStream > SharedMemoryEmscripten::read(bool exclusive)
{
	return new MemoryStream(m_data.ptr(), m_size, true, false);
}

Ref< IStream > SharedMemoryEmscripten::write()
{
	return new MemoryStream(m_data.ptr(), m_size, false, true);
}

}
