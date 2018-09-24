/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
