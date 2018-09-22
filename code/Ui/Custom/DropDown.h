/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_DropDown_H
#define traktor_ui_custom_DropDown_H

#include <string>
#include "Ui/Widget.h"

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

// class Button;
// class Edit;
// class ToolForm;

		namespace custom
		{

// class ListBox;

/*! \brief Drop down control.
 * \ingroup UIC
 */
class T_DLLCLASS DropDown : public Widget
{
	T_RTTI_CLASS;

public:
	DropDown();

	bool create(Widget* parent, int style = WsNone);

	int32_t add(const std::wstring& item, Object* data = 0);

	bool remove(int32_t index);

	void removeAll();

	int32_t count() const;

	void setItem(int32_t index, const std::wstring& item);

	void setData(int32_t index, Object* data);

	std::wstring getItem(int32_t index) const;

	Ref< Object > getData(int32_t index) const;

	void select(int32_t index);

	bool select(const std::wstring& item);

	int32_t getSelected() const;

	std::wstring getSelectedItem() const;

	Ref< Object > getSelectedData() const;

	template < typename T >
	Ref< T > getData(int32_t index) const
	{
		return dynamic_type_cast< T* >(getData(index));
	}

	template < typename T >
	Ref< T > getSelectedData() const
	{
		return dynamic_type_cast< T* >(getSelectedData());
	}

	virtual Size getPreferedSize() const T_OVERRIDE;

private:
	struct Item
	{
		std::wstring text;
		Ref< Object > data;
	};

	std::vector< Item > m_items;
	int32_t m_selected;

	void eventMouseMove(MouseMoveEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_DropDown_H
