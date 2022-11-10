/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ShaderGraph;
class Node;

class T_DLLCLASS ShaderGraphValidator : public Object
{
	T_RTTI_CLASS;

public:
	enum ShaderGraphType
	{
		SgtShader,
		SgtProgram,
		SgtFragment
	};

	explicit ShaderGraphValidator(const ShaderGraph* shaderGraph);

	explicit ShaderGraphValidator(const ShaderGraph* shaderGraph, const Guid& shaderGraphId);

	bool validate(ShaderGraphType type, std::vector< const Node* >* outErrorNodes = 0) const;

	bool validateIntegrity() const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	Guid m_shaderGraphId;
};

	}
}

