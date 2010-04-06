#ifndef traktor_ui_custom_AutoWidgetCell_H
#define traktor_ui_custom_AutoWidgetCell_H

#include "Core/Object.h"
#include "Ui/Associative.h"
#include "Ui/Rect.h"

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

class Canvas;

		namespace custom
		{

class AutoWidget;

/*! \brief Auto widget cell.
 * \ingroup UIC
 */
class T_DLLCLASS AutoWidgetCell
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	virtual void placeCells(AutoWidget* widget, const Rect& rect);

	virtual AutoWidgetCell* hitTest(AutoWidget* widget, const Point& position);

	virtual bool beginCapture(AutoWidget* widget);

	virtual void endCapture(AutoWidget* widget);

	virtual void mouseDown(AutoWidget* widget, const Point& position);

	virtual void mouseUp(AutoWidget* widget, const Point& position);

	virtual void mouseMove(AutoWidget* widget, const Point& position);

	virtual void paint(AutoWidget* widget, Canvas& canvas, const Rect& rect);
};

		}
	}
}

#endif	// traktor_ui_custom_AutoWidgetCell_H
