#include "Core/Misc/Murmur3.h"
#include "Render/Editor/Glsl/GlslSampler.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslSampler", GlslSampler, GlslResource)

GlslSampler::GlslSampler(const std::wstring& name, uint8_t stages, const SamplerState& state, const std::wstring& textureName)
:	GlslResource(name, stages)
,	m_state(state)
,	m_textureName(textureName)
{
}

int32_t GlslSampler::getOrdinal() const
{
	Murmur3 cs;
	cs.begin();
	cs.feed(getName());
	cs.end();
	return (int32_t)cs.get();
}

}
