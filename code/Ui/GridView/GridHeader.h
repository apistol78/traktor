#pragma once

#include "Core/RefArray.h"
#include "Ui/Auto/AutoWidgetCell.h"

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

class GridColumn;

class T_DLLCLASS GridHeader : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	void setColumns(const RefArray< GridColumn >& columns);

	virtual void mouseDown(MouseButtonDownEvent* event, const Point& position) override final;

	virtual void mouseUp(MouseButtonUpEvent* event, const Point& position) override final;

	virtual void mouseMove(MouseMoveEvent* event, const Point& position) override final;

	virtual void paint(Canvas& canvas, const Rect& rect) override final;

private:
	RefArray< GridColumn > m_columns;
	Ref< GridColumn > m_resizeColumn;
	int32_t m_resizeWidth;
	int32_t m_resizePosition;
};

	}
}

