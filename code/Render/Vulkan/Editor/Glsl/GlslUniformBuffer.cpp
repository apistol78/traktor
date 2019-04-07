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

bool GlslUniformBuffer::add(const std::wstring& uniformName, GlslType uniformType, int32_t length)
{
	// If adding already existing uniform then type and length must match.
	auto it = std::find_if(m_uniforms.begin(), m_uniforms.end(), [&](const Uniform& uniform) {
		return uniform.name == uniformName;
	});
	if (it != m_uniforms.end())
	{
		if (it->type == uniformType && it->length == length)
			return true;
		else
			return false;
	}

	// No such uniform; add new uniform.
	m_uniforms.push_back({ uniformName, uniformType, length });

	// Always keep list sorted by type first then name.
	std::stable_sort(m_uniforms.begin(), m_uniforms.end(), [](const Uniform& lh, const Uniform& rh) {
		return lh.name > rh.name;
	});
	std::stable_sort(m_uniforms.begin(), m_uniforms.end(), [](const Uniform& lh, const Uniform& rh) {
		return lh.type > rh.type;
	});

	return true;
}

	}
}