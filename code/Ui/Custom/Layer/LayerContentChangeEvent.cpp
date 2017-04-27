/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/Layer/LayerContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.LayerContentChangeEvent", LayerContentChangeEvent, ContentChangeEvent)

LayerContentChangeEvent::LayerContentChangeEvent(EventSubject* sender, LayerItem* item)
:	ContentChangeEvent(sender)
,	m_item(item)
{
}

LayerItem* LayerContentChangeEvent::getItem() const
{
	return m_item;
}

		}
	}
}
