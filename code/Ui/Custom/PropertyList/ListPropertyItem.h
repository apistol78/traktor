/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ListPropertyItem_H
#define traktor_ui_custom_ListPropertyItem_H

#include <vector>
#include "Ui/Custom/PropertyList/PropertyItem.h"

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

class ToolForm;
class ListBox;

		namespace custom
		{

class MiniButton;

/*! \brief List property item.
 * \ingroup UIC
 */
class T_DLLCLASS ListPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	ListPropertyItem(const std::wstring& text);

	virtual ~ListPropertyItem();

	int add(const std::wstring& item);

	bool remove(int index);

	void removeAll();

	int count() const;

	std::wstring get(int index) const;

	void select(int index);

	int getSelected() const;

	std::wstring getSelectedItem() const;

protected:
	virtual void createInPlaceControls(Widget* parent) T_OVERRIDE;

	virtual void destroyInPlaceControls() T_OVERRIDE;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) T_OVERRIDE;

	virtual void paintValue(Canvas& canvas, const Rect& rc) T_OVERRIDE;

private:
	std::vector< std::wstring > m_items;
	int32_t m_selected;
	Ref< MiniButton > m_buttonDrop;
	Ref< ToolForm > m_listForm;
	Ref< ListBox > m_listBox;
	Rect m_listRect;

	void eventDropClick(ButtonClickEvent* event);

	void eventSelect(SelectionChangeEvent* event);

	void eventFocus(FocusEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ListPropertyItem_H
