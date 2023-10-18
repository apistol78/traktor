/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Render/Types.h"
#include "Render/Editor/Glsl/GlslResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{
	
class T_DLLCLASS GlslSampler : public GlslResource
{
	T_RTTI_CLASS;

public:
	explicit GlslSampler(const std::wstring& name, Set set, uint8_t stages, const SamplerState& state, const std::wstring& textureName = L"");

	const SamplerState& getState() const { return m_state; }

	const std::wstring& getTextureName() const { return m_textureName; }

	virtual int32_t getOrdinal() const override final;

private:
	SamplerState m_state;
	std::wstring m_textureName;	//!< Bound texture with sampler, only used in OpenGL since samplers and textures are bound.
};

}
