/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ListViewItem_H
#define traktor_ui_ListViewItem_H

#include <map>
#include <string>
#include <vector>
#include "Core/Object.h"
#include "Ui/Associative.h"

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

/*! \brief List view item.
 * \ingroup UI
 */
class T_DLLCLASS ListViewItem
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	void setText(int columnIndex, const std::wstring& text);

	std::wstring getText(int columnIndex) const;
	
	void setImage(int columnIndex, int imageIndex);
	
	int getImage(int columnIndex) const;

	int getColumnCount() const;

private:
	struct ItemData
	{
		std::wstring text;
		int image;

		ItemData()
		:	image(-1)
		{
		}
	};
	std::vector< ItemData > m_items;
};

	}
}

#endif	// traktor_ui_ListViewItem_H
