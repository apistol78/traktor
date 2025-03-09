/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/Image2/ImagePassStep.h"
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

class ITexture;
class Shader;

/*!
 * \ingroup Render
 */
class T_DLLCLASS AmbientOcclusion : public ImagePassStep
{
	T_RTTI_CLASS;

public:
	virtual void addRenderPassInputs(
		const ImageGraph* graph,
		const ImageGraphContext& context,
		RenderPass& pass) const override final;

	virtual void build(
		const ImageGraph* graph,
		const ImageGraphContext& context,
		const ImageGraphView& view,
		const targetSetVector_t& targetSetIds,
		const bufferVector_t& sbufferIds,
		const PassOutput& output,
		const RenderGraph& renderGraph,
		const ProgramParameters* sharedParams,
		RenderContext* renderContext,
		ScreenRenderer* screenRenderer) const override final;

private:
	friend class AmbientOcclusionData;

	resource::Proxy< render::Shader > m_shader;
	Vector4 m_offsets[64];
	Vector4 m_directions[8];
	Ref< ITexture > m_randomNormals;
	Ref< ITexture > m_randomRotations;
};

}
