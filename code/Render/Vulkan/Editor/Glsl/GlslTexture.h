#pragma once

#include "Render/Vulkan/Editor/Glsl/GlslResource.h"
#include "Render/Vulkan/Editor/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{
	
class GlslTexture : public GlslResource
{
	T_RTTI_CLASS;

public:
	explicit GlslTexture(const std::wstring& name, GlslType uniformType);

	GlslType getUniformType() const { return m_uniformType; }

	virtual int32_t getOrdinal() const override final;

private:
	GlslType m_uniformType;
};

	}
}