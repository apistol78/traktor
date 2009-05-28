#include "Ui/Custom/GridView/GridItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridItem", GridItem, Object)

GridItem::GridItem()
:	m_image(-1)
,	m_expandedImage(-1)
{
}

GridItem::GridItem(const std::wstring& text)
:	m_text(text)
,	m_image(-1)
,	m_expandedImage(-1)
{
}

GridItem::GridItem(const std::wstring& text, int32_t image)
:	m_text(text)
,	m_image(image)
,	m_expandedImage(image)
{
}

GridItem::GridItem(const std::wstring& text, int32_t image, int32_t expandedImage)
:	m_text(text)
,	m_image(image)
,	m_expandedImage(expandedImage)
{
}

void GridItem::setText(const std::wstring& text)
{
	m_text = text;
}

void GridItem::setImage(int32_t image)
{
	m_image = image;
}

void GridItem::setExpandedImage(int expandedImage)
{
	m_expandedImage = expandedImage;
}

		}
	}
}
