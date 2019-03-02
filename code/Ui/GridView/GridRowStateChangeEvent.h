#pragma once

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

