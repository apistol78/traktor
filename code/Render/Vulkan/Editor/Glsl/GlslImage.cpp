#include "Render/Vulkan/Editor/Glsl/GlslImage.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslImage", GlslImage, GlslResource)

GlslImage::GlslImage(const std::wstring& name)
:	GlslResource(name)
{
}

	}
}