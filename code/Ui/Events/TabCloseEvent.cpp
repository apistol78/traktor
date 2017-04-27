/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/TabCloseEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TabCloseEvent", TabCloseEvent, CloseEvent)

TabCloseEvent::TabCloseEvent(EventSubject* sender, TabPage* tabPage)
:	CloseEvent(sender)
,	m_tabPage(tabPage)
{
}

TabPage* TabCloseEvent::getTabPage() const
{
	return m_tabPage;
}

	}
}
