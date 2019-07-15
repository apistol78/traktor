#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslResource.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslType.h"

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

	GlslUniformBuffer(const std::wstring& name, int32_t binding);

	bool add(const std::wstring& uniformName, GlslType uniformType, int32_t length);

	int32_t getBinding() const { return m_binding; }

	const AlignedVector< Uniform >& get() const { return m_uniforms; }

private:
	int32_t m_binding;
	AlignedVector< Uniform > m_uniforms;
};

	}
}