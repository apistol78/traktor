#pragma once

#include "Render/Vulkan/Editor/Glsl/GlslResource.h"
#include "Render/Vulkan/Editor/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{
	
class GlslImage : public GlslResource
{
	T_RTTI_CLASS;

public:
	explicit GlslImage(const std::wstring& name, uint8_t stages);

	virtual int32_t getOrdinal() const override final;
};

	}
}