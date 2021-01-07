#include "Render/Vulkan/VertexBufferVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferVk", VertexBufferVk, VertexBuffer)

VertexBufferVk::VertexBufferVk(
	uint32_t bufferSize,
	const VkVertexInputBindingDescription& vertexBindingDescription,
	const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
	uint32_t hash,
	uint32_t& instances
)
:	VertexBuffer(bufferSize)
,	m_vertexBindingDescription(vertexBindingDescription)
,	m_vertexAttributeDescriptions(vertexAttributeDescriptions)
,	m_hash(hash)
,	m_instances(instances)
{
	Atomic::increment((int32_t&)m_instances);
}

VertexBufferVk::~VertexBufferVk()
{
	Atomic::decrement((int32_t&)m_instances);
}

	}
}
