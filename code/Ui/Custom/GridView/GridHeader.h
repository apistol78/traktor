#ifndef traktor_ui_custom_GridHeader_H
#define traktor_ui_custom_GridHeader_H

#include "Core/RefArray.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

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

class GridColumn;

class T_DLLCLASS GridHeader : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	void setColumns(const RefArray< GridColumn >& columns);

	virtual void mouseDown(MouseButtonDownEvent* event, const Point& position) T_OVERRIDE T_FINAL;

	virtual void mouseUp(MouseButtonUpEvent* event, const Point& position) T_OVERRIDE T_FINAL;

	virtual void mouseMove(MouseMoveEvent* event, const Point& position) T_OVERRIDE T_FINAL;

	virtual void paint(Canvas& canvas, const Rect& rect) T_OVERRIDE T_FINAL;

private:
	RefArray< GridColumn > m_columns;
	Ref< GridColumn > m_resizeColumn;
	int32_t m_resizeWidth;
	int32_t m_resizePosition;
};

		}
	}
}

#endif	// traktor_ui_custom_GridHeader_H
