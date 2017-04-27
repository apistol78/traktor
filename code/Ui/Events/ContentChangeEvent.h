/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ContentChangeEvent_H
#define traktor_ui_ContentChangeEvent_H

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
	
/*! \brief Command event.
 * \ingroup UI
 */
class T_DLLCLASS ContentChangeEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	ContentChangeEvent(EventSubject* sender);
};
	
	}
}

#endif	// traktor_ui_ContentChangeEvent_H
