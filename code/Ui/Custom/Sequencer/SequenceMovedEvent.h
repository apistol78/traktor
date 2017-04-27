/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_SequenceMovedEvent_H
#define traktor_ui_custom_SequenceMovedEvent_H

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class SequenceItem;

/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS SequenceMovedEvent : public Event
{
	T_RTTI_CLASS;

public:
	SequenceMovedEvent(EventSubject* sender, SequenceItem* item, int32_t movedTo);

	SequenceItem* getItem() const;

	int32_t getMovedTo() const;

private:
	Ref< SequenceItem > m_item;
	int32_t m_movedTo;
};

		}
	}
}

#endif	// traktor_ui_custom_SequenceMovedEvent_H
