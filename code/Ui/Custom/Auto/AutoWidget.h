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

	void setFocusCell(AutoWidgetCell* focusCell);

	AutoWidgetCell* getFocusCell() const;

	AutoWidgetCell* hitTest(const Point& position);

	void requestUpdate();

	void requestLayout();

	void placeCell(AutoWidgetCell* cell, const Rect& rc);

	virtual void layoutCells(const Rect& rc) = 0;

private:
	struct CellInstance
	{
		Ref< AutoWidgetCell > cell;
		Rect rc;
	};
	std::vector< CellInstance > m_cells;
	Ref< AutoWidgetCell > m_focusCell;
	Ref< AutoWidgetCell > m_captureCell;
	Ref< ScrollBar > m_scrollBar;
	Color m_backgroundColor;
	Size m_scrollOffset;
	Rect m_bounds;
	bool m_deferredUpdate;
	bool m_deferredLayout;

	void updateLayout();

	void eventButtonDown(Event* event);

	void eventButtonUp(Event* event);

	void eventMouseMove(Event* event);

	void eventMouseWheel(Event* event);

	void eventPaint(Event* event);

	void eventSize(Event* event);

	void eventTimer(Event* event);

	void eventScroll(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_AutoWidget_H
