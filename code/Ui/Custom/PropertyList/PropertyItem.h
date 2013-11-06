#ifndef traktor_ui_custom_PropertyItem_H
#define traktor_ui_custom_PropertyItem_H

#include <string>
#include "Core/Object.h"
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

class Canvas;
class Rect;
class Command;
class MouseEvent;

		namespace custom
		{

class PropertyList;

/*! \brief Property item.
 * \ingroup UIC
 */
class T_DLLCLASS PropertyItem : public Object
{
	T_RTTI_CLASS;

public:
	PropertyItem(const std::wstring& text);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void expand();

	void collapse();

	bool isExpanded() const;

	bool isCollapsed() const;

	void setSelected(bool selected);

	bool isSelected() const;

	int getDepth() const;

	PropertyItem* getParentItem() const;

	RefArray< PropertyItem >& getChildItems();

	const RefArray< PropertyItem >& getChildItems() const;

protected:
	friend class AutoPropertyList;
	friend class PropertyList;

	void setPropertyList(PropertyList* propertyList);

	PropertyList* getPropertyList() const;

	void notifyUpdate();

	void notifyCommand(const Command& command);

	void notifyChange();

	void addChildItem(PropertyItem* childItem);

	void removeChildItem(PropertyItem* childItem);

	virtual void createInPlaceControls(Widget* parent);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void mouseButtonDown(MouseEvent* event);

	virtual void mouseButtonUp(MouseEvent* event);

	virtual void doubleClick(MouseEvent* event);

	virtual void mouseMove(MouseEvent* event);

	virtual void paintBackground(Canvas& canvas, const Rect& rc);

	virtual void paintText(Canvas& canvas, const Rect& rc);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

	virtual bool copy();

	virtual bool paste();

private:
	PropertyList* m_propertyList;
	std::wstring m_text;
	bool m_expanded;
	bool m_selected;
	PropertyItem* m_parent;
	RefArray< PropertyItem > m_childItems;

	void updateChildrenInPlaceControls();
};

		}
	}
}

#endif	// traktor_ui_custom_PropertyItem_H
