/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Node;
class ShaderGraph;

/*! Shader graph hash.
 * \ingroup Render
 * 
 * Since shader graphs can be expressed in multiple ways
 * but yield same result we cannot rely on data-only hash.
 * This class is designed to calculate an analytical hash
 * of the shader graph's function, i.e. handle commutative
 * inputs etc.
 */
class T_DLLCLASS ShaderGraphHash : public Object
{
	T_RTTI_CLASS;

public:
	explicit ShaderGraphHash(bool includeTextures);

	uint32_t calculate(const Node* node) const;

	uint32_t calculate(const ShaderGraph* shaderGraph) const;

private:
	bool m_includeTextures;	//!< Include texture node id in hash.
};

}
