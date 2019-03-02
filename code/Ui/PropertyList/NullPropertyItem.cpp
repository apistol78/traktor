#include "Ui/PropertyList/NullPropertyItem.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NullPropertyItem", NullPropertyItem, PropertyItem)

NullPropertyItem::NullPropertyItem()
:	PropertyItem(L"")
{
}

void NullPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
}

	}
}
