/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ToolBarButtonClickEvent_H
#define traktor_ui_custom_ToolBarButtonClickEvent_H

#include "Ui/Events/ButtonClickEvent.h"

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

class ToolBarItem;

/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS ToolBarButtonClickEvent : public ButtonClickEvent
{
	T_RTTI_CLASS;

public:
	ToolBarButtonClickEvent(EventSubject* sender, ToolBarItem* item, const Command& command);

	ToolBarItem* getItem() const;

private:
	Ref< ToolBarItem > m_item;
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarButtonClickEvent_H
