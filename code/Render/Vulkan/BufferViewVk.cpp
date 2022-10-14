#include "Render/Vulkan/BufferViewVk.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.BufferViewVk", BufferViewVk, IBufferView)

BufferViewVk::BufferViewVk(VkBuffer buffer, uint32_t offset, uint32_t range, uint32_t size)
:	m_buffer(buffer)
,	m_offset(offset)
,	m_range(range)
,	m_size(size)
{
}

}
