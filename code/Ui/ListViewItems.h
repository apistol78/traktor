#ifndef traktor_ui_ListViewItems_H
#define traktor_ui_ListViewItems_H

#include <vector>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

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

class ListViewItem;

/*! \brief Collection of list view items.
 * \ingroup UI
 */
class T_DLLCLASS ListViewItems : public Object
{
	T_RTTI_CLASS(ListViewItems)

public:
	void add(ListViewItem* item);

	void remove(ListViewItem* item);

	int count() const;

	ListViewItem* get(int index) const;

private:
	RefArray< ListViewItem > m_items;
};

	}
}

#endif	// traktor_ui_ListViewItems_H
