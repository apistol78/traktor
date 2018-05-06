/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ListViewItems_H
#define traktor_ui_ListViewItems_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

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

class ListViewItem;

/*! \brief Collection of list view items.
 * \ingroup UI
 */
class T_DLLCLASS ListViewItems : public Object
{
	T_RTTI_CLASS;

public:
	void add(ListViewItem* item);

	void remove(ListViewItem* item);

	void removeAll();

	int count() const;

	Ref< ListViewItem > get(int index) const;

private:
	RefArray< ListViewItem > m_items;
};

	}
}

#endif	// traktor_ui_ListViewItems_H
