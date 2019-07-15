#include "Render/OpenGL/Std/Editor/Glsl/GlslSampler.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslSampler.GL", GlslSampler, GlslResource)

GlslSampler::GlslSampler(const std::wstring& name, int32_t unit, const SamplerState& state, const std::wstring& textureName)
:	GlslResource(name)
,	m_unit(unit)
,	m_state(state)
,	m_textureName(textureName)
{
}

	}
}