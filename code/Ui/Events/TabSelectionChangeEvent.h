#pragma once

#include "Ui/Events/SelectionChangeEvent.h"

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

class TabPage;

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS TabSelectionChangeEvent : public SelectionChangeEvent
{
	T_RTTI_CLASS;

public:
	explicit TabSelectionChangeEvent(EventSubject* sender, TabPage* tabPage);

	TabPage* getTabPage() const;

private:
	Ref< TabPage > m_tabPage;
};

	}
}

