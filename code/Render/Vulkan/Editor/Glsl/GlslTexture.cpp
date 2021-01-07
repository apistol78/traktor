#include "Core/Misc/Adler32.h"
#include "Render/Vulkan/Editor/Glsl/GlslTexture.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslTexture", GlslTexture, GlslResource)

GlslTexture::GlslTexture(const std::wstring& name, uint8_t stages, GlslType uniformType)
:	GlslResource(name, stages)
,	m_uniformType(uniformType)
{
}

int32_t GlslTexture::getOrdinal() const
{
	Adler32 cs;
	cs.begin();
	cs.feed(getName());
	cs.end();
	return (int32_t)cs.get();
}

	}
}