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

ImageGraphContext::TextureTargetSet ImageGraphContext::findTextureTargetSet(handle_t textureId) const
{
	auto it = m_textureTargetSet.find(textureId);
	if (it != m_textureTargetSet.end())
		return it->second;
	else
	{
		const static TextureTargetSet s_invalid = { 0, 0 };
		return s_invalid;
	}
}
	
	}
}