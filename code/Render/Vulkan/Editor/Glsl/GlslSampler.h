#pragma once

#include <string>
#include "Render/Types.h"
#include "Render/Vulkan/Editor/Glsl/GlslResource.h"

namespace traktor
{
	namespace render
	{
	
class GlslSampler : public GlslResource
{
	T_RTTI_CLASS;

public:
	GlslSampler(const std::wstring& name, const SamplerState& state);

	const SamplerState& getState() const { return m_state; }

private:
	SamplerState m_state;
};

	}
}