#include "Render/Vulkan/StructBufferVk.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferVk", StructBufferVk, StructBuffer)

void StructBufferVk::destroy()
{
}

void* StructBufferVk::lock()
{
	return nullptr;
}

void* StructBufferVk::lock(uint32_t structOffset, uint32_t structCount)
{
	return nullptr;
}

void StructBufferVk::unlock()
{
}

	}
}