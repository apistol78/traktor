#pragma once

#include <string>
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
	GlslTexture(const std::wstring& name, GlslType uniformType);

	const std::wstring& getName() const { return m_name; }

	GlslType getUniformType() const { return m_uniformType; }

private:
	std::wstring m_name;
	GlslType m_uniformType;
};

	}
}