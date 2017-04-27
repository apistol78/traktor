/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_EvtHandler_H
#define traktor_ui_EvtHandler_H

#include <wx/wx.h>

namespace traktor
{
	namespace ui
	{
		namespace
		{

template < typename TargetType, typename wxEventType >
class EventAdapter : public wxEvtHandler
{
public:
	typedef void (TargetType::*MethodType)(wxEventType& event);
	
	EventAdapter(TargetType* target, MethodType method)
	:	m_target(target)
	,	m_method(method)
	{
	}
	
	void dispatch(wxEventType& event)
	{
		(m_target->*m_method)(event);
	}
	
private:
	TargetType* m_target;
	MethodType m_method;
};

		}

#define T_CONNECT(window, eventId, eventType, targetType, targetMethod) \
	{ \
		(window)->Connect( \
			eventId, \
			(wxObjectEventFunction)&EventAdapter< targetType, eventType >::dispatch, \
			NULL, \
			new EventAdapter< targetType, eventType >(this, targetMethod) \
		); \
	}

	}
}

#endif	// traktor_ui_EvtHandler_H
