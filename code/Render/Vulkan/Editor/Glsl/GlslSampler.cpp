#include "Render/Vulkan/Editor/Glsl/GlslSampler.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslSampler", GlslSampler, GlslResource)

GlslSampler::GlslSampler(const std::wstring& name, const SamplerState& state)
:	m_name(name)
,	m_state(state)
{
}

	}
}