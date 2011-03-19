#include "World/Editor/PostProcess/PostProcessStepItem.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepItem", PostProcessStepItem, Object)

PostProcessStepItem::PostProcessStepItem(PostProcessStepItem* parent, PostProcessStep* step, const std::wstring& text, int32_t image)
:	m_parent(parent)
,	m_step(step)
,	m_text(text)
,	m_image(image)
{
}

PostProcessStepItem* PostProcessStepItem::getParent() const
{
	return m_parent;
}

PostProcessStep* PostProcessStepItem::getStep() const
{
	return m_step;
}

const std::wstring& PostProcessStepItem::getText() const
{
	return m_text;
}

int32_t PostProcessStepItem::getImage() const
{
	return m_image;
}

	}
}
