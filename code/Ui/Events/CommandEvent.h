/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_CommandEvent_H
#define traktor_ui_CommandEvent_H

#include "Ui/Event.h"
#include "Ui/Command.h"

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
class T_DLLCLASS CommandEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	CommandEvent(EventSubject* sender, const Command& command);

	CommandEvent(EventSubject* sender);
	
	const Command& getCommand() const;
	
private:
	Command m_command;
};
	
	}
}

#endif	// traktor_ui_CommandEvent_H
