/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS UniformDeclaration : public ISerializable
{
	T_RTTI_CLASS;

public:
	ParameterType getParameterType() const { return m_type; }

	int32_t getLength() const { return m_length; }

	UpdateFrequency getFrequency() const { return m_frequency; }

	virtual void serialize(ISerializer& s) override final;

private:
	friend class ListUniformsTool;

	ParameterType m_type = ParameterType::Scalar;
	int32_t m_length = 0;
	UpdateFrequency m_frequency = UpdateFrequency::Once;
};

}
