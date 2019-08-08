#pragma once

#include <string>
#include <vector>
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

/*! Drop down control.
 * \ingroup UI
 */
class T_DLLCLASS DropDown : public Widget
{
	T_RTTI_CLASS;

public:
	enum Styles
	{
		WsSingle = 0,
		WsMultiple = WsUser,
		WsDefault = WsSingle
	};

	DropDown();

	bool create(Widget* parent, int32_t style = WsDefault);

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

	void unselect(int32_t index);

	bool selected(int32_t index) const;

	int32_t getSelected() const;

	int32_t getSelected(std::vector< int32_t >& selected) const;

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

	virtual Size getPreferedSize() const override;

private:
	struct Item
	{
		std::wstring text;
		Ref< Object > data;
		bool selected;
	};

	std::vector< Item > m_items;
	bool m_multiple;
	bool m_hover;

	void eventMouseTrack(MouseTrackEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

