#include "Ui/Custom/GridView/GridItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridItem", GridItem, Object)

GridItem::GridItem(const std::wstring& text)
:	m_text(text)
{
}

void GridItem::setText(const std::wstring& text)
{
	m_text = text;
}

		}
	}
}
