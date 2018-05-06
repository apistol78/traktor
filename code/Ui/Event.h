/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Event_H
#define traktor_ui_Event_H

#include "Core/Object.h"
#include "Core/Ref.h"

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

class EventSubject;

/*! \brief Event base class.
 * \ingroup UI
 *
 * All UI events must be derived from this class
 * in order to be passed through the event mechanism.
 * When this class is instantiated a snapshot of the
 * keyboard state is taken as often different
 * actions is performed by holding various "modifier"
 * keys.
 */
class T_DLLCLASS Event : public Object
{
	T_RTTI_CLASS;
	
public:
	Event(EventSubject* sender);

	EventSubject* getSender() const;

	int getKeyState() const;

	void consume();
	
	bool consumed() const;
	
private:
	Ref< EventSubject > m_sender;
	int m_keyState;
	bool m_consumed;
};

	}
}

#endif	// traktor_ui_Event_H
