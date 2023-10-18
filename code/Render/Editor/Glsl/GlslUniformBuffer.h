/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Editor/Glsl/GlslResource.h"
#include "Render/Editor/Glsl/GlslType.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{
	
class T_DLLCLASS GlslUniformBuffer : public GlslResource
{
	T_RTTI_CLASS;

public:
	struct Uniform
	{
		std::wstring name;
		GlslType type;
		int32_t length;
	};

	explicit GlslUniformBuffer(const std::wstring& name, Set set, uint8_t stages);

	bool add(const std::wstring& uniformName, GlslType uniformType, int32_t length);

	const AlignedVector< Uniform >& get() const { return m_uniforms; }

	virtual int32_t getOrdinal() const override final { return 0; }

private:
	AlignedVector< Uniform > m_uniforms;
};

}
