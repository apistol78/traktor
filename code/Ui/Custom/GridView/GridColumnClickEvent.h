#ifndef traktor_ui_custom_GridColumnClickEvent_H
#define traktor_ui_custom_GridColumnClickEvent_H

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

class T_DLLCLASS GridColumnClickEvent : public Event
{
	T_RTTI_CLASS;

public:
	GridColumnClickEvent(EventSubject* sender, GridRow* row, int32_t column);

	GridRow* getRow() const;

	int32_t getColumn() const;

private:
	Ref< GridRow > m_row;
	int32_t m_column;
};

		}
	}
}

#endif	// traktor_ui_custom_GridColumnClickEvent_H
