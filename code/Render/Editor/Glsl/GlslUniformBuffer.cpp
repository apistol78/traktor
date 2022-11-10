/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Render/Editor/Glsl/GlslUniformBuffer.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslUniformBuffer", GlslUniformBuffer, GlslResource)

GlslUniformBuffer::GlslUniformBuffer(const std::wstring& name, uint8_t stages, int32_t ordinal)
:	GlslResource(name, stages)
,	m_ordinal(ordinal)
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

	// Always keep list sorted by length, type then name.
	std::stable_sort(m_uniforms.begin(), m_uniforms.end(), [](const Uniform& lh, const Uniform& rh) {
		return lh.name > rh.name;
	});
	std::stable_sort(m_uniforms.begin(), m_uniforms.end(), [](const Uniform& lh, const Uniform& rh) {
		return lh.type > rh.type;
	});
	std::stable_sort(m_uniforms.begin(), m_uniforms.end(), [](const Uniform& lh, const Uniform& rh) {
		return lh.length > rh.length;
	});

	return true;
}

}
