/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/GridView/GridItemContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GridItemContentChangeEvent", GridItemContentChangeEvent, ContentChangeEvent)

GridItemContentChangeEvent::GridItemContentChangeEvent(EventSubject* sender, GridItem* item)
:	ContentChangeEvent(sender)
,	m_item(item)
{
}

GridItem* GridItemContentChangeEvent::getItem() const
{
	return m_item;
}

		}
	}
}
