#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Vulkan/Editor/Glsl/GlslResource.h"
#include "Render/Vulkan/Editor/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{
	
class GlslUniformBuffer : public GlslResource
{
	T_RTTI_CLASS;

public:
	struct Uniform
	{
		std::wstring name;
		GlslType type;
		int32_t length;
	};

	GlslUniformBuffer(const std::wstring& name);

	bool add(const std::wstring& uniformName, GlslType uniformType, int32_t length);

	const AlignedVector< Uniform >& get() const { return m_uniforms; }

private:
	AlignedVector< Uniform > m_uniforms;
};

	}
}