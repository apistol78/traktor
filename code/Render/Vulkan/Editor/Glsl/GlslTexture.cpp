#include "Render/Vulkan/Editor/Glsl/GlslTexture.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslTexture", GlslTexture, GlslResource)

GlslTexture::GlslTexture(const std::wstring& name, GlslType uniformType)
:	m_name(name)
,	m_uniformType(uniformType)
{
}

	}
}