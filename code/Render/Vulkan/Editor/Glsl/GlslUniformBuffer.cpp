#include <algorithm>
#include "Render/Vulkan/Editor/Glsl/GlslUniformBuffer.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslUniformBuffer", GlslUniformBuffer, GlslResource)

GlslUniformBuffer::GlslUniformBuffer(const std::wstring& name)
:	GlslResource(name)
{
}

void GlslUniformBuffer::add(const std::wstring& uniformName, GlslType uniformType, int32_t length)
{
	m_uniforms.push_back({ uniformName, uniformType, length });
	std::stable_sort(m_uniforms.begin(), m_uniforms.end(), [](const Uniform& lh, const Uniform& rh) {
		return lh.name > rh.name;
	});
	std::stable_sort(m_uniforms.begin(), m_uniforms.end(), [](const Uniform& lh, const Uniform& rh) {
		return lh.type > rh.type;
	});
}

	}
}