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
#include "Core/Ref.h"

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

/*! Static analysis on shader graphs.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphStatic : public Object
{
	T_RTTI_CLASS;

public:
	explicit ShaderGraphStatic(const ShaderGraph* shaderGraph, const Guid& shaderGraphId);

	/*! Get permutation of shader graph for given platform. */
	Ref< ShaderGraph > getPlatformPermutation(const std::wstring& platform) const;

	/*! Get permutation of shader graph for given renderer. */
	Ref< ShaderGraph > getRendererPermutation(const std::wstring& renderer) const;

	/*! Replace all "Connected" nodes with direct connections. */
	Ref< ShaderGraph > getConnectedPermutation() const;

	/*! Replace all "Type" nodes with direct connections based on input type. */
	Ref< ShaderGraph > getTypePermutation() const;

	/*! Insert swizzle nodes for all inputs to ensure widths are as small as possible. */
	Ref< ShaderGraph > getSwizzledPermutation() const;

	/*! Calculate constant branches and replace with simpler branches. */
	Ref< ShaderGraph > getConstantFolded() const;

	/*! Remove redundant swizzle nodes. */
	Ref< ShaderGraph > cleanupRedundantSwizzles() const;

	/*! Propagate state given as input into PixelOutput. */
	Ref< ShaderGraph > getStateResolved() const;

	/*! Replace bundle nodes with direct connections. */
	Ref< ShaderGraph > getBundleResolved() const;

	/*! Replace variable nodes with direct connections. */
	Ref< ShaderGraph > getVariableResolved() const;

	/*! Remove disabled outputs. */
	Ref< ShaderGraph > removeDisabledOutputs() const;

	/*! Set values in external nodes from constant inputs. */
	Ref< ShaderGraph > propagateConstantExternalValues() const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	Guid m_shaderGraphId;
};

	}
}
