#ifndef traktor_ui_custom_AutoWidgetCell_H
#define traktor_ui_custom_AutoWidgetCell_H

#include "Core/Object.h"
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
class T_DLLCLASS AutoWidgetCell : public Object
{
	T_RTTI_CLASS;

public:
	virtual void setRect(const Rect& rect);

	virtual Rect getRect() const;

	virtual AutoWidgetCell* hitTest(AutoWidget* widget, const Point& position);

	virtual bool beginCapture(AutoWidget* widget);

	virtual void endCapture(AutoWidget* widget);

	virtual void mouseDown(AutoWidget* widget, const Point& position);

	virtual void mouseUp(AutoWidget* widget, const Point& position);

	virtual void mouseMove(AutoWidget* widget, const Point& position);

	virtual void paint(AutoWidget* widget, Canvas& canvas, const Size& offset);

private:
	Rect m_rect;
};

		}
	}
}

#endif	// traktor_ui_custom_AutoWidgetCell_H
