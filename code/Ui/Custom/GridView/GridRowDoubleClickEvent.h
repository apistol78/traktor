/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_GridRowDoubleClickEvent_H
#define traktor_ui_custom_GridRowDoubleClickEvent_H

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

class T_DLLCLASS GridRowDoubleClickEvent : public Event
{
	T_RTTI_CLASS;

public:
	GridRowDoubleClickEvent(EventSubject* sender, GridRow* row, int32_t columnIndex);

	GridRow* getRow() const;

	int32_t getColumnIndex() const;

private:
	Ref< GridRow > m_row;
	int32_t m_columnIndex;
};

		}
	}
}

#endif	// traktor_ui_custom_GridRowDoubleClickEvent_H
