/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_TabCloseEvent_H
#define traktor_ui_TabCloseEvent_H

#include "Ui/Events/CloseEvent.h"

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

class TabPage;
	
/*! \brief Close event.
 * \ingroup UI
 */
class T_DLLCLASS TabCloseEvent : public CloseEvent
{
	T_RTTI_CLASS;
	
public:
	TabCloseEvent(EventSubject* sender, TabPage* tabPage);

	TabPage* getTabPage() const;

private:
	Ref< TabPage > m_tabPage;
};
	
	}
}

#endif	// traktor_ui_TabCloseEvent_H
