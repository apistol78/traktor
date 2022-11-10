#include "Core/Misc/Murmur3.h"
#include "Render/Editor/Glsl/GlslTexture.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslTexture", GlslTexture, GlslResource)

GlslTexture::GlslTexture(const std::wstring& name, uint8_t stages, GlslType uniformType)
:	GlslResource(name, stages)
,	m_uniformType(uniformType)
{
}

int32_t GlslTexture::getOrdinal() const
{
	Murmur3 cs;
	cs.begin();
	cs.feed(getName());
	cs.end();
	return (int32_t)cs.get();
}

}
