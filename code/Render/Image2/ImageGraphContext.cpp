/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/IRenderTargetSet.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraphContext.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageGraphContext", ImageGraphContext, Object)

void ImageGraphContext::associateTexture(img_handle_t textureId, ITexture* texture)
{
	auto& txts = m_textureTargetSet[textureId];
	txts.targetSetId = 0;
	txts.colorIndex = -1;
	txts.texture = texture;
}

void ImageGraphContext::associateTextureTargetSet(img_handle_t textureId, handle_t targetSetId, int32_t colorIndex)
{
	auto& txts = m_textureTargetSet[textureId];
	txts.targetSetId = targetSetId;
	txts.colorIndex = colorIndex;
	txts.texture = nullptr;
}

void ImageGraphContext::associateTextureTargetSetDepth(img_handle_t textureId, handle_t targetSetId)
{
	auto& txts = m_textureTargetSet[textureId];
	txts.targetSetId = targetSetId;
	txts.colorIndex = -1;
	txts.texture = nullptr;
}

handle_t ImageGraphContext::findTextureTargetSetId(img_handle_t textureId) const
{
	auto it = m_textureTargetSet.find(textureId);
	if (it != m_textureTargetSet.end())
		return it->second.targetSetId;
	else
		return 0;
}

ITexture* ImageGraphContext::findTexture(const RenderGraph& renderGraph, img_handle_t textureId) const
{
	auto it = m_textureTargetSet.find(textureId);
	if (it == m_textureTargetSet.end())
		return nullptr;

	if (it->second.texture)
		return it->second.texture;

	auto targetSet = renderGraph.getTargetSet(it->second.targetSetId);
	if (!targetSet)
		return nullptr;

	if (it->second.colorIndex >= 0)
		return targetSet->getColorTexture(it->second.colorIndex);
	else
		return targetSet->getDepthTexture();
}

void ImageGraphContext::associateSBuffer(img_handle_t sbufferId, handle_t frameSbufferId)
{
	m_sbufferHandles[sbufferId] = frameSbufferId;
}

handle_t ImageGraphContext::findSBufferId(img_handle_t sbufferId) const
{
	auto it = m_sbufferHandles.find(sbufferId);
	if (it != m_sbufferHandles.end())
		return it->second;
	else
		return 0;
}

Buffer* ImageGraphContext::findSBuffer(const RenderGraph& renderGraph, img_handle_t sbufferId) const
{
	auto it = m_sbufferHandles.find(sbufferId);
	if (it == m_sbufferHandles.end())
		return nullptr;

	return renderGraph.getBuffer(it->second);
}

//void ImageGraphContext::setParameters(const std::function< void(ProgramParameters*) >& parametersFn)
//{
//	m_parametersFn = parametersFn;
//}

//void ImageGraphContext::setFloatParameter(handle_t handle, float value)
//{
//	m_scalarParameters[handle] = value;
//}
//
//void ImageGraphContext::setVectorParameter(handle_t handle, const Vector4& value)
//{
//	m_vectorParameters[handle] = value;
//}
//
//void ImageGraphContext::setTextureParameter(handle_t handle, ITexture* texture)
//{
//	m_textureParameters[handle] = texture;
//}

}
