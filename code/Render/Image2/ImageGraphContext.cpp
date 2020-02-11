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

void ImageGraphContext::associateTextureTargetSet(handle_t textureId, handle_t targetSetId, int32_t colorIndex)
{
	auto& txts = m_textureTargetSet[textureId];
	txts.targetSetId = targetSetId;
	txts.colorIndex = colorIndex;
}

void ImageGraphContext::associateTextureTargetSetDepth(handle_t textureId, handle_t targetSetId)
{
	auto& txts = m_textureTargetSet[textureId];
	txts.targetSetId = targetSetId;
	txts.colorIndex = -1;
}

handle_t ImageGraphContext::findTextureTargetSetId(handle_t textureId) const
{
	auto it = m_textureTargetSet.find(textureId);
	if (it != m_textureTargetSet.end())
		return it->second.targetSetId;
	else
		return 0;
}

ITexture* ImageGraphContext::findTexture(const RenderGraph& renderGraph, handle_t textureId) const
{
	auto it = m_textureTargetSet.find(textureId);
	if (it == m_textureTargetSet.end())
		return nullptr;

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