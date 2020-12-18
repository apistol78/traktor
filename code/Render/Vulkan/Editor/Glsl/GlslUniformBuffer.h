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

	explicit GlslUniformBuffer(const std::wstring& name, int32_t ordinal);

	bool add(const std::wstring& uniformName, GlslType uniformType, int32_t length);

	const AlignedVector< Uniform >& get() const { return m_uniforms; }

	virtual int32_t getOrdinal() const override final { return m_ordinal; }

private:
	AlignedVector< Uniform > m_uniforms;
	int32_t m_ordinal;
};

	}
}