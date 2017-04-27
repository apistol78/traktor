/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_FocusEvent_H
#define traktor_ui_FocusEvent_H

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Focus event.
 * \ingroup UI
 */
class T_DLLCLASS FocusEvent : public Event
{
	T_RTTI_CLASS;

public:
	FocusEvent(EventSubject* sender, bool gotFocus);

	bool gotFocus() const;

	bool lostFocus() const;

private:
	bool m_gotFocus;
};

	}
}

#endif	// traktor_ui_FocusEvent_H
