#ifndef traktor_ui_custom_PreviewSelectionChangeEvent_H
#define traktor_ui_custom_PreviewSelectionChangeEvent_H

#include "Ui/Events/SelectionChangeEvent.h"

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

class PreviewItem;

/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS PreviewSelectionChangeEvent : public SelectionChangeEvent
{
	T_RTTI_CLASS;
	
public:
	PreviewSelectionChangeEvent(EventSubject* sender, PreviewItem* item);
	
	PreviewItem* getItem() const;

private:
	Ref< PreviewItem > m_item;
};

		}
	}
}

#endif	// traktor_ui_custom_PreviewSelectionChangeEvent_H
