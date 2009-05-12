#ifndef traktor_ui_custom_GridDragEvent_H
#define traktor_ui_custom_GridDragEvent_H

#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class GridRow;

/*! \brief Grid row drag event.
 * \ingroup UI
 */
class T_DLLCLASS GridDragEvent : public Event
{
	T_RTTI_CLASS(GridDragEvent)
	
public:
	GridDragEvent(EventSubject* sender, GridRow* dragItem, int dropRow);

	int getDropRow() const;

	void cancel();

	bool cancelled() const;

private:
	int m_dropRow;
	bool m_cancelled;
};

		}
	}
}

#endif	// traktor_ui_custom_GridDragEvent_H
