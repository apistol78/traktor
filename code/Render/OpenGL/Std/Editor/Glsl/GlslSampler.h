#pragma once

#include "Render/Types.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslResource.h"

namespace traktor
{
	namespace render
	{
	
class GlslSampler : public GlslResource
{
	T_RTTI_CLASS;

public:
	GlslSampler(const std::wstring& name, int32_t unit, const SamplerState& state, const std::wstring& textureName);

	int32_t getUnit() const { return m_unit; }

	const SamplerState& getState() const { return m_state; }

	const std::wstring& getTextureName() const { return m_textureName; }

private:
	int32_t m_unit;
	SamplerState m_state;
	std::wstring m_textureName;
};

	}
}