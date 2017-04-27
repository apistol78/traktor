/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/Tab/TabSelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.TabSelectionChangeEvent", TabSelectionChangeEvent, SelectionChangeEvent)

TabSelectionChangeEvent::TabSelectionChangeEvent(EventSubject* sender, TabPage* tabPage)
:	SelectionChangeEvent(sender)
,	m_tabPage(tabPage)
{
}

TabPage* TabSelectionChangeEvent::getTabPage() const
{
	return m_tabPage;
}

		}
	}
}
