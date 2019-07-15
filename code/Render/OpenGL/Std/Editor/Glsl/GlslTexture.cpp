#include "Render/OpenGL/Std/Editor/Glsl/GlslTexture.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslTexture.GL", GlslTexture, GlslResource)

GlslTexture::GlslTexture(const std::wstring& name, GlslType uniformType)
:	GlslResource(name)
,	m_uniformType(uniformType)
{
}

	}
}