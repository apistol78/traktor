#include "Render/Vulkan/Editor/Glsl/GlslResource.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslResource", GlslResource, Object)
	
GlslResource::GlslResource()
:	m_binding(-1)
{
}

	}
}