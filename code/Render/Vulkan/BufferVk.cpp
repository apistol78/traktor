#include "Core/Thread/Atomic.h"
#include "Render/Vulkan/BufferVk.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferVk", BufferVk, Buffer)

BufferVk::BufferVk(Context* context, uint32_t elementCount, uint32_t elementSize, uint32_t& instances)
:	Buffer(elementCount, elementSize)
,	m_context(context)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

BufferVk::~BufferVk()
{
	Atomic::decrement((int32_t&)m_instances);
}

}
