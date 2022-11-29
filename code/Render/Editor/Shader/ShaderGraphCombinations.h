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
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ShaderGraph;

/*! Shader graph combination generator.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphCombinations : public Object
{
	T_RTTI_CLASS;

public:
	struct Combination
	{
		uint32_t mask;
		uint32_t value;
		Ref< const ShaderGraph > shaderGraph;
	};

	explicit ShaderGraphCombinations(const ShaderGraph* shaderGraph, const Guid& shaderGraphId);

	const AlignedVector< std::wstring >& getParameterNames() const;

	AlignedVector< std::wstring > getParameterNames(uint32_t mask) const;

	uint32_t getCombinationCount() const;

	uint32_t getCombinationMask(uint32_t index) const;

	uint32_t getCombinationValue(uint32_t index) const;

	Ref< const ShaderGraph > getCombinationShaderGraph(uint32_t index) const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	AlignedVector< std::wstring > m_parameterNames;
	AlignedVector< Combination > m_combinations;
};

}
