/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ShowEvent_H
#define traktor_ui_ShowEvent_H

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

/*! \brief Show event.
 * \ingroup UI
 */
class T_DLLCLASS ShowEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	ShowEvent(EventSubject* sender, bool visible);
	
	bool isVisible() const;
	
private:
	bool m_visible;
};

	}
}

#endif	// traktor_ui_ShowEvent_H
