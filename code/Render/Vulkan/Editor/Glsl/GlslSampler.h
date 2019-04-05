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

	const std::wstring& getName() const { return m_name; }

	const SamplerState& getState() const { return m_state; }

private:
	std::wstring m_name;
	SamplerState m_state;
};

	}
}