#pragma once

#include "Ui/Events/ContentChangeEvent.h"

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

class LayerItem;

/*! \brief
 * \ingroup UI
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

