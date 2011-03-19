#include "World/Editor/PostProcess/PostProcessDefineItem.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessDefineItem", PostProcessDefineItem, Object)

PostProcessDefineItem::PostProcessDefineItem(PostProcessDefine* definition, const std::wstring& text, int32_t image)
:	m_definition(definition)
,	m_text(text)
,	m_image(image)
{
}

PostProcessDefine* PostProcessDefineItem::getDefinition() const
{
	return m_definition;
}

const std::wstring& PostProcessDefineItem::getText() const
{
	return m_text;
}

int32_t PostProcessDefineItem::getImage() const
{
	return m_image;
}

	}
}
