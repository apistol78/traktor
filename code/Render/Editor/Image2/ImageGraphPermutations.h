/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"
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

class ImageGraphAsset;

/*! Image graph permutation generator.
 * \ingroup Render
 */
class T_DLLCLASS ImageGraphPermutations : public Object
{
	T_RTTI_CLASS;

public:
	struct Permutation
	{
		uint32_t mask;
		uint32_t value;
		Ref< const ImageGraphAsset > imageGraph;
	};

	explicit ImageGraphPermutations(const ImageGraphAsset* imageGraph);

	const AlignedVector< std::wstring >& getParameterNames() const;

	AlignedVector< std::wstring > getParameterNames(uint32_t mask) const;

	uint32_t getPermutationCount() const;

	uint32_t getPermutationMask(uint32_t index) const;

	uint32_t getPermutationValue(uint32_t index) const;

	Ref< const ImageGraphAsset > getPermutationImageGraph(uint32_t index) const;

private:
	Ref< const ImageGraphAsset > m_imageGraph;
	AlignedVector< std::wstring > m_parameterNames;
	AlignedVector< Permutation > m_permutations;
};

}
