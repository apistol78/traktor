#include "Render/Vulkan/Editor/Glsl/GlslUniformBuffer.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslUniformBuffer", GlslUniformBuffer, GlslResource)

GlslUniformBuffer::GlslUniformBuffer(const std::wstring& name)
:	m_name(name)
{
}

void GlslUniformBuffer::add(const std::wstring& uniformName, GlslType uniformType, int32_t length)
{
	m_uniforms.push_back({ uniformName, uniformType, length });
}

	}
}