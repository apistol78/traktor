#ifndef traktor_ui_TreeViewDragEvent_H
#define traktor_ui_TreeViewDragEvent_H

#include "Ui/Event.h"
#include "Ui/Point.h"

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

class TreeViewItem;

/*! \brief Tree view drag event.
 * \ingroup UI
 */
class T_DLLCLASS TreeViewDragEvent : public Event
{
	T_RTTI_CLASS;
	
public:
	enum DragMoment
	{
		DmDrag,
		DmDrop
	};

	TreeViewDragEvent(EventSubject* sender, TreeViewItem* dragItem, DragMoment moment, const Point& position = Point(0, 0));

	TreeViewItem* getItem() const;

	DragMoment getMoment() const;

	const Point& getPosition() const;

	void cancel();

	bool cancelled() const;

private:
	Ref< TreeViewItem > m_dragItem;
	DragMoment m_moment;
	Point m_position;
	bool m_cancelled;
};
	
	}
}

#endif	// traktor_ui_TreeViewDragEvent_H
