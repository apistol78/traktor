#include "Ui/Custom/PropertyList/NullPropertyItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.NullPropertyItem", NullPropertyItem, PropertyItem)

NullPropertyItem::NullPropertyItem()
:	PropertyItem(L"")
{
}

void NullPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
}

		}
	}
}
