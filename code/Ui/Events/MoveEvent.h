/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_MoveEvent_H
#define traktor_ui_MoveEvent_H

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
	
/*! \brief Move event.
 * \ingroup UI
 */
class T_DLLCLASS MoveEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	MoveEvent(EventSubject* sender, const Point& position);
	
	const Point& getPosition() const;
	
private:
	Point m_position;
};
	
	}
}

#endif	// traktor_ui_MoveEvent_H
