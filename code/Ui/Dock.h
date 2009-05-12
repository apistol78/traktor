#ifndef traktor_ui_Dock_H
#define traktor_ui_Dock_H

#include "Core/Heap/Ref.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(Dock)

public:
	bool create(Widget* parent);

	virtual void destroy();

	DockPane* getPane();

	virtual void update(const Rect* rc = 0, bool immediate = false);

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

	void eventSize(Event* event);

	void eventButtonDown(Event* event);

	void eventButtonUp(Event* event);

	void eventMouseMove(Event* event);

	void eventDoubleClick(Event* event);

	void eventPaint(Event* event);

	void eventFormMove(Event* event);

	void eventFormNcButtonUp(Event* event);

	void eventHintButtonUp(Event* event);
};

	}
}

#endif	// traktor_ui_Dock_H
