/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/StaticVector.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ImageGraph;
class ImageGraphContext;
class ImagePassStep;
class RenderGraph;
class ScreenRenderer;

struct ImageGraphView;

/*! Image pass
 * \ingroup Render
 * 
 * An image pass represent an entire render pass,
 * so an image pass may contain several steps which
 * are executed in sequence in a single render pass.
 */
class T_DLLCLASS ImagePass : public Object
{
	T_RTTI_CLASS;

public:
	typedef StaticVector< handle_t, 32 > targetSetVector_t;

	void addRenderGraphPasses(
		const ImageGraph* graph,
		const ImageGraphContext& context,
		const ImageGraphView& view,
		const targetSetVector_t& targetSetIds,
		ScreenRenderer* screenRenderer,
		RenderGraph& renderGraph
	) const;

private:
	friend class ImagePassData;

	std::wstring m_name;
	int32_t m_outputTargetSet;
	Clear m_clear;
	RefArray< const ImagePassStep > m_steps;
};

}
