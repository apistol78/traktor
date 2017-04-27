/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/ContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ContentChangeEvent", ContentChangeEvent, Event)

ContentChangeEvent::ContentChangeEvent(EventSubject* sender)
:	Event(sender)
{
}

	}
}
