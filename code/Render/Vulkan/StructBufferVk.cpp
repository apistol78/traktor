#include "Render/Vulkan/StructBufferVk.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferVk", StructBufferVk, StructBuffer)

StructBufferVk::StructBufferVk(Context* context, uint32_t bufferSize, uint32_t& instances)
:	StructBuffer(bufferSize)
,	m_context(context)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

StructBufferVk::~StructBufferVk()
{
	Atomic::decrement((int32_t&)m_instances);
}

	}
}