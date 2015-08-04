#ifndef traktor_ui_custom_GridCellContentChangeEvent_H
#define traktor_ui_custom_GridCellContentChangeEvent_H

#include "Ui/Events/ContentChangeEvent.h"

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

class GridCell;
	
/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS GridCellContentChangeEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;
	
public:
	GridCellContentChangeEvent(EventSubject* sender, GridCell* item);

	GridCell* getItem() const;

private:
	Ref< GridCell > m_item;
};
	
		}
	}
}

#endif	// traktor_ui_custom_GridCellContentChangeEvent_H
