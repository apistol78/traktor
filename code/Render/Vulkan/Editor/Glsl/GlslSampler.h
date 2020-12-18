#pragma once

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
	explicit GlslSampler(const std::wstring& name, const SamplerState& state);

	const SamplerState& getState() const { return m_state; }

	virtual int32_t getOrdinal() const override final;

private:
	SamplerState m_state;
};

	}
}