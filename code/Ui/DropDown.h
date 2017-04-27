/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_DropDown_H
#define traktor_ui_DropDown_H

#include <map>
#include "Ui/Widget.h"

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

/*! \brief DropDown
 * \ingroup UI
 */
class T_DLLCLASS DropDown : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text = L"", int32_t style = WsBorder);
	
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

private:
	std::map< int32_t, Ref< Object > > m_data;
};

	}
}

#endif	// traktor_ui_DropDown_H
