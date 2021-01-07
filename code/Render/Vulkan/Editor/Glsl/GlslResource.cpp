#include "Render/Vulkan/Editor/Glsl/GlslResource.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslResource", GlslResource, Object)
	
GlslResource::GlslResource(const std::wstring& name, uint8_t stages)
:	m_binding(-1)
,	m_stages(stages)
,	m_name(name)
{
}

	}
}