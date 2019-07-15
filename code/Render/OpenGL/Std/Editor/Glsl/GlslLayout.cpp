#include "Render/OpenGL/Std/Editor/Glsl/GlslLayout.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslResource.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslLayout.GL", GlslLayout, Object)

void GlslLayout::add(GlslResource* resource)
{
	m_resources.push_back(resource);
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