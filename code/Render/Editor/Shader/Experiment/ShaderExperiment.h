/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class SxData;

class T_DLLCLASS ShaderExperiment : public ISerializable
{
	T_RTTI_CLASS;

public:
	enum class Initial
	{
		Undefined,
		Zero,
		Random
	};

	struct Pass
	{
		std::wstring technique;
		int32_t workSize = 1;

		void serialize(ISerializer& s);
	};

	const Guid& getShader() const { return m_shader; }

	const RefArray< SxData >& getData() const { return m_data; }

	const AlignedVector< Pass >& getPasses() const { return m_passes; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_shader;
	RefArray< SxData > m_data;
	AlignedVector< Pass > m_passes;
};

}
