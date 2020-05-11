#include "Render/IRenderTargetSet.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraphContext.h"

namespace traktor
{
	namespace render
	{

ImageGraphContext::ImageGraphContext(ScreenRenderer* screenRenderer)
:	m_screenRenderer(screenRenderer)
{
}

void ImageGraphContext::associateTexture(handle_t textureId, ITexture* texture)
{
	auto& txts = m_textureTargetSet[textureId];
	txts.targetSetId = 0;
	txts.colorIndex = -1;
	txts.texture = texture;
}

void ImageGraphContext::associateTextureTargetSet(handle_t textureId, handle_t targetSetId, int32_t colorIndex)
{
	auto& txts = m_textureTargetSet[textureId];
	txts.targetSetId = targetSetId;
	txts.colorIndex = colorIndex;
	txts.texture = nullptr;
}

void ImageGraphContext::associateTextureTargetSetDepth(handle_t textureId, handle_t targetSetId)
{
	auto& txts = m_textureTargetSet[textureId];
	txts.targetSetId = targetSetId;
	txts.colorIndex = -1;
	txts.texture = nullptr;
}

std::pair< handle_t, bool > ImageGraphContext::findTextureTargetSetId(handle_t textureId) const
{
	auto it = m_textureTargetSet.find(textureId);
	if (it != m_textureTargetSet.end())
		return { it->second.targetSetId, (bool)(it->second.colorIndex < 0) };
	else
		return { 0, false };
}

ITexture* ImageGraphContext::findTexture(const RenderGraph& renderGraph, handle_t textureId) const
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

	}
}