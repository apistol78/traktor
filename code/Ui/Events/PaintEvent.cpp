/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/PaintEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PaintEvent", PaintEvent, Event)

PaintEvent::PaintEvent(EventSubject* sender, Canvas& canvas, const Rect& rc)
:	Event(sender)
,	m_canvas(canvas)
,	m_rc(rc)
{
}

Canvas& PaintEvent::getCanvas() const
{
	return m_canvas;
}

const Rect& PaintEvent::getUpdateRect() const
{
	return m_rc;
}

	}
}
