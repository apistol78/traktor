/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_MouseButtonUpEvent_H
#define traktor_ui_MouseButtonUpEvent_H

#include "Ui/Event.h"
#include "Ui/Point.h"

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
	
/*! \brief Mouse event.
 * \ingroup UI
 */
class T_DLLCLASS MouseButtonUpEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	MouseButtonUpEvent(EventSubject* sender, int32_t button, const ui::Point& position);
	
	int32_t getButton() const;
	
	const ui::Point& getPosition() const;
	
private:
	int32_t m_button;
	ui::Point m_position;
};
	
	}
}

#endif	// traktor_ui_MouseButtonUpEvent_H
