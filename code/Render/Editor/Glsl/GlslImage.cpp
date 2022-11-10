#include "Core/Misc/Murmur3.h"
#include "Render/Editor/Glsl/GlslImage.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslImage", GlslImage, GlslResource)

GlslImage::GlslImage(const std::wstring& name, uint8_t stages)
:	GlslResource(name, stages)
{
}

int32_t GlslImage::getOrdinal() const
{
	Murmur3 cs;
	cs.begin();
	cs.feed(getName());
	cs.end();
	return (int32_t)cs.get();
}

}
