#ifndef traktor_ui_Dock_H
#define traktor_ui_Dock_H

#include "Ui/Widget.h"

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

class DockPane;
class ToolForm;
class Image;

/*! \brief Docking widget.
 * \ingroup UI
 */
class T_DLLCLASS Dock : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent);

	virtual void destroy() T_OVERRIDE;

	DockPane* getPane();

	void dumpLayout();

	virtual void update(const Rect* rc = 0, bool immediate = false) T_OVERRIDE;

private:
	Ref< DockPane > m_pane;
	Ref< DockPane > m_splittingPane;
	Ref< DockPane > m_hintDockPane;
	Ref< ToolForm > m_hintDockForm;
	Ref< ToolForm > m_hint;
	Ref< Image > m_hintLeft;
	Ref< Image > m_hintRight;
	Ref< Image > m_hintTop;
	Ref< Image > m_hintBottom;

	void eventSize(SizeEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventPaint(PaintEvent* event);

	void eventFormMove(MoveEvent* event);

	void eventFormNcButtonUp(NcMouseButtonUpEvent* event);

	void eventHintButtonUp(MouseButtonUpEvent* event);
};

	}
}

#endif	// traktor_ui_Dock_H
