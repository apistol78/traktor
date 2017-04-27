/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/PropertyList/PropertyCommandEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.PropertyCommandEvent", PropertyCommandEvent, CommandEvent)

PropertyCommandEvent::PropertyCommandEvent(EventSubject* sender, PropertyItem* item, const Command& command)
:	CommandEvent(sender, command)
,	m_item(item)
{
}

PropertyItem* PropertyCommandEvent::getItem() const
{
	return m_item;
}

		}
	}
}
