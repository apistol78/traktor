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
#include "Render/Types.h"
#include "Resource/Proxy.h"

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
class ProgramParameters;
class RenderContext;
class RenderGraph;
class RenderPass;
class ScreenRenderer;
class Shader;

struct ImageGraphView;

/*! Image pass step.
 * \ingroup Render
 * 
 * Each pass can have a sequence of operations
 * to implement it's functionality.
 */
class T_DLLCLASS ImagePassStep : public Object
{
	T_RTTI_CLASS;

public:
	/*! */
	virtual void addRenderPassInputs(
		const ImageGraph* graph,
		const ImageGraphContext& context,
		RenderPass& pass
	) const = 0;

	/*! */
	virtual void build(
		const ImageGraph* graph,
		const ImageGraphContext& context,
		const ImageGraphView& view,
		const RenderGraph& renderGraph,
		const ProgramParameters* sharedParams,
		RenderContext* renderContext,
		ScreenRenderer* screenRenderer
	) const = 0;

protected:
	friend class ImagePassStepData;

	struct Source
	{
		handle_t textureId;
		handle_t parameter;
	};

	resource::Proxy< render::Shader > m_shader;
	AlignedVector< Source > m_sources;    
};

}
