#include "Core/System/Emscripten/SharedMemoryEmscripten.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.SharedMemoryEmscripten", SharedMemoryEmscripten, ISharedMemory)

SharedMemoryEmscripten::SharedMemoryEmscripten(uint32_t size)
:	m_data(new uint8_t [size])
,	m_size(size)
{
}

const void* SharedMemoryEmscripten::acquireReadPointer(bool exclusive)
{
	return m_data.c_ptr();
}

void SharedMemoryEmscripten::releaseReadPointer()
{
}

void* SharedMemoryEmscripten::acquireWritePointer()
{
	return m_data.ptr();
}

void SharedMemoryEmscripten::releaseWritePointer()
{
}

}
