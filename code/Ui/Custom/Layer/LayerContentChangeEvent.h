#ifndef traktor_ui_custom_LayerContentChangeEvent_H
#define traktor_ui_custom_LayerContentChangeEvent_H

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

class LayerItem;

/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS LayerContentChangeEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;
	
public:
	LayerContentChangeEvent(EventSubject* sender, LayerItem* item);
	
	LayerItem* getItem() const;

private:
	Ref< LayerItem > m_item;
};

		}
	}
}

#endif	// traktor_ui_custom_LayerContentChangeEvent_H
