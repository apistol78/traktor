/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Image2/ImagePassStep.h"

#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImagePassStep", ImagePassStep, Object)

void ImagePassStep::addInputs(const ImageGraphContext& context, RenderPass& pass) const
{
	for (const auto& source : m_textureSources)
	{
		const RGTargetSet targetSetId = context.findTextureTargetSetId(source.id);
		if (targetSetId != RGTargetSet::Invalid)
			pass.addInput(targetSetId);
	}
	for (const auto& source : m_sbufferSources)
	{
		const RGBuffer sbufferId = context.findSBufferId(source.id);
		if (sbufferId != RGBuffer::Invalid)
			pass.addInput(sbufferId);
	}
}

void ImagePassStep::bindSources(const ImageGraphContext& context, const RenderGraph& renderGraph, ProgramParameters* programParameters) const
{
	for (const auto& source : m_textureSources)
	{
		auto texture = context.findTexture(renderGraph, source.id);
		programParameters->setTextureParameter(source.shaderParameter, texture);
	}
	for (const auto& source : m_sbufferSources)
	{
		auto sbuffer = context.findSBuffer(renderGraph, source.id);
		programParameters->setBufferViewParameter(source.shaderParameter, (sbuffer != nullptr) ? sbuffer->getBufferView() : nullptr);
	}
}

}
