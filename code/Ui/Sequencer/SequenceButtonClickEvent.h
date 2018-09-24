/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_SequenceButtonClickEvent_H
#define traktor_ui_SequenceButtonClickEvent_H

#include "Ui/Events/ButtonClickEvent.h"

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

class Sequence;

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS SequenceButtonClickEvent : public ButtonClickEvent
{
	T_RTTI_CLASS;

public:
	SequenceButtonClickEvent(EventSubject* sender, Sequence* sequence, const Command& command);

	Sequence* getSequence() const;

private:
	Ref< Sequence > m_sequence;
};

	}
}

#endif	// traktor_ui_SequenceButtonClickEvent_H
