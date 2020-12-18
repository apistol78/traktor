#include "Core/Misc/Adler32.h"
#include "Render/Vulkan/Editor/Glsl/GlslSampler.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslSampler", GlslSampler, GlslResource)

GlslSampler::GlslSampler(const std::wstring& name, const SamplerState& state)
:	GlslResource(name)
,	m_state(state)
{
}

int32_t GlslSampler::getOrdinal() const
{
	Adler32 cs;
	cs.begin();
	cs.feed(getName());
	cs.end();
	return (int32_t)cs.get();
}

	}
}