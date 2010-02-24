#ifndef traktor_ui_custom_AutoWidget_H
#define traktor_ui_custom_AutoWidget_H

#include "Ui/Widget.h"

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

class ScrollBar;

		namespace custom
		{

class AutoWidgetCell;

/*! \brief Auto layout widget.
 * \ingroup UIC
 *
 * AutoWidget simplifies creation of custom widgets
 * by managing pieces of the widget through AutoWidgetCell classes.
 * The class automates hot-item tracking, mouse capture and
 * animation updates.
 */
class T_DLLCLASS AutoWidget : public Widget
{
	T_RTTI_CLASS;

public:
	AutoWidget();

	bool create(ui::Widget* parent, int32_t style);

	void setBackgroundColor(const Color& color);

	void setClientSize(const Size& size);

	void setFocusCell(AutoWidgetCell* focusCell);

	AutoWidgetCell* getFocusCell() const;

	AutoWidgetCell* hitTest(const Point& position);

	void requestUpdate();

	void requestLayout();

protected:
	void addCell(AutoWidgetCell* cell);

	void removeAllCells();

	const RefArray< AutoWidgetCell >& getCells() const;

	virtual void layoutCells(const Rect& rc) = 0;

private:
	RefArray< AutoWidgetCell > m_cells;
	Ref< AutoWidgetCell > m_focusCell;
	Ref< AutoWidgetCell > m_captureCell;
	Ref< ScrollBar > m_scrollBar;
	Color m_backgroundColor;
	Size m_scrollOffset;
	Size m_clientSize;
	bool m_deferredUpdate;

	void updateScrollBar();

	void updateLayout();

	void eventButtonDown(Event* event);

	void eventButtonUp(Event* event);

	void eventMouseMove(Event* event);

	void eventPaint(Event* event);

	void eventSize(Event* event);

	void eventTimer(Event* event);

	void eventScroll(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_AutoWidget_H
