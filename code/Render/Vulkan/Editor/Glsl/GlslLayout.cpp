#include "Render/Vulkan/Editor/Glsl/GlslLayout.h"
#include "Render/Vulkan/Editor/Glsl/GlslResource.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslLayout", GlslLayout, Object)

void GlslLayout::add(GlslResource* resource)
{
	T_ASSERT(resource->m_binding == -1);
	resource->m_binding = (int32_t)m_resources.size();
	m_resources.push_back(resource);
}

GlslResource* GlslLayout::get(int32_t binding)
{
	return m_resources[binding];
}

uint32_t GlslLayout::count(const TypeInfo& resourceType) const
{
	uint32_t c = 0;
	for (auto resource : m_resources)
	{
		if (is_type_a(resourceType, type_of(resource)))
			++c;
	}
	return c;
}

	}
}