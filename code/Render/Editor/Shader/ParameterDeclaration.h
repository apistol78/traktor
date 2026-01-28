/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Render/Types.h"
#include "Render/Editor/Shader/StructDeclaration.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS ParameterDeclaration : public ISerializable
{
	T_RTTI_CLASS;

public:
	ParameterType getParameterType() const { return m_type; }

	int32_t getLength() const { return m_length; }

	UpdateFrequency getFrequency() const { return m_frequency; }

	const std::wstring& getStructType() const { return m_structType; }

	const StructDeclaration& getStructDeclaration() const { return m_declaration; }

	virtual void serialize(ISerializer& s) override final;

private:
	friend class TouchShaderGraphsTool;
	friend class ParameterLinker;

	ParameterType m_type = ParameterType::Scalar;
	int32_t m_length = 0;
	UpdateFrequency m_frequency = UpdateFrequency::Once;
	Guid m_structDeclaration;

	//!\group Linker
	//!\{
	std::wstring m_structType;
	StructDeclaration m_declaration;
	//!\}
};

}
