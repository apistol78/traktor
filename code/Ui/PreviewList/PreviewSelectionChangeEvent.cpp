/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/PreviewList/PreviewSelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PreviewSelectionChangeEvent", PreviewSelectionChangeEvent, SelectionChangeEvent)

PreviewSelectionChangeEvent::PreviewSelectionChangeEvent(EventSubject* sender, PreviewItem* item)
:	SelectionChangeEvent(sender)
,	m_item(item)
{
}

PreviewItem* PreviewSelectionChangeEvent::getItem() const
{
	return m_item;
}

	}
}
