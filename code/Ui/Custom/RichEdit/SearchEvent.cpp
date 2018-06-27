/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/RichEdit/SearchEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.SearchEvent", SearchEvent, ui::Event)

SearchEvent::SearchEvent(
	ui::EventSubject* sender,
	bool preview
)
:	ui::Event(sender)
,	m_preview(preview)
{
}

		}
	}
}
