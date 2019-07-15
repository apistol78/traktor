#include "Render/OpenGL/Std/Editor/Glsl/GlslResource.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslResource.GL", GlslResource, Object)
	
GlslResource::GlslResource(const std::wstring& name)
:	m_name(name)
{
}

	}
}