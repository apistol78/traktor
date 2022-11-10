#include "Render/Editor/Glsl/GlslLayout.h"
#include "Render/Editor/Glsl/GlslResource.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslResource", GlslResource, Object)
	
GlslResource::GlslResource(const std::wstring& name, uint8_t stages)
:	m_name(name)
,	m_stages(stages)
{
}

int32_t GlslResource::getBinding(GlslDialect dialect) const
{
	if (dialect == GlslDialect::OpenGL)
		return m_layout->getLocalIndex(this);
	else if (dialect == GlslDialect::Vulkan)
		return m_layout->getGlobalIndex(this);
	else
		return -1;
}

}
