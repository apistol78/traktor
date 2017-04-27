/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_GridRowStateChangeEvent_H
#define traktor_ui_custom_GridRowStateChangeEvent_H

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

class GridRow;

class T_DLLCLASS GridRowStateChangeEvent : public Event
{
	T_RTTI_CLASS;

public:
	GridRowStateChangeEvent(EventSubject* sender, GridRow* row);

	GridRow* getRow() const;

private:
	Ref< GridRow > m_row;
};

		}
	}
}

#endif	// traktor_ui_custom_GridRowStateChangeEvent_H
