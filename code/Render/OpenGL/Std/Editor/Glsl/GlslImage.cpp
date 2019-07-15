#include "Render/OpenGL/Std/Editor/Glsl/GlslImage.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslImage.GL", GlslImage, GlslResource)

GlslImage::GlslImage(const std::wstring& name)
:	GlslResource(name)
{
}

	}
}