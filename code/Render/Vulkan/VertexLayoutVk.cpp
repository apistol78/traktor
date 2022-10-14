#include "Render/Vulkan/VertexLayoutVk.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexLayoutVk", VertexLayoutVk, IVertexLayout)

VertexLayoutVk::VertexLayoutVk(
	const VkVertexInputBindingDescription& vertexBindingDescription,
	const AlignedVector< VkVertexInputAttributeDescription >& vertexAttributeDescriptions,
	uint32_t hash
)
:	m_vertexBindingDescription(vertexBindingDescription)
,	m_vertexAttributeDescriptions(vertexAttributeDescriptions)
,	m_hash(hash)
{
}

}
