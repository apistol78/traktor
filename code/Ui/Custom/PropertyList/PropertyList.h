#ifndef traktor_ui_custom_PropertyList_H
#define traktor_ui_custom_PropertyList_H

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

class Guid;

	namespace ui
	{

class HierarchicalState;
class ScrollBar;

		namespace custom
		{

class PropertyItem;

/*! \brief Property list control.
 * \ingroup UIC
 */
class T_DLLCLASS PropertyList : public Widget
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsColumnHeader = WsUser
	};

	enum GetPropertyItemFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfSelectedOnly = 2,
		GfExpandedOnly = 4
	};

	struct IPropertyGuidResolver
	{
		virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const = 0;
	};

	PropertyList();

	bool create(Widget* parent, int style = WsDoubleBuffer, IPropertyGuidResolver* guidResolver = 0);

	virtual void destroy();

	void addPropertyItem(PropertyItem* propertyItem);

	void removePropertyItem(PropertyItem* propertyItem);

	void addPropertyItem(PropertyItem* parentPropertyItem, PropertyItem* propertyItem);

	void removePropertyItem(PropertyItem* parentPropertyItem, PropertyItem* propertyItem);

	void removeAllPropertyItems();

	int getPropertyItems(RefArray< PropertyItem >& propertyItems, int flags);

	void setSeparator(int separator);

	int getSeparator() const;

	void setColumnName(int column, const std::wstring& name);

	Ref< PropertyItem > getPropertyItemFromPosition(const Point& position);

	bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;

	Ref< HierarchicalState > captureState() const;

	void applyState(const HierarchicalState* state);

	virtual bool copy();

	virtual bool paste();

	void addSelectEventHandler(EventHandler* eventHandler);

	void addCommandEventHandler(EventHandler* eventHandler);
	
	void addChangeEventHandler(EventHandler* eventHandler);

	virtual void update(const Rect* rc = 0, bool immediate = false);

	virtual Size getMinimumSize() const;
	
	virtual Size getPreferedSize() const;

private:
	friend class PropertyItem;

	IPropertyGuidResolver* m_guidResolver;
	Ref< ScrollBar > m_scrollBar;
	RefArray< PropertyItem > m_propertyItems;
	Ref< PropertyItem > m_mousePropertyItem;
	int m_separator;
	int m_mode;
	bool m_columnHeader;
	std::wstring m_columnNames[2];

	void updateScrollBar();

	void placeItems();

	void eventScroll(Event* event);

	void eventButtonDown(Event* event);

	void eventButtonUp(Event* event);

	void eventDoubleClick(Event* event);

	void eventMouseMove(Event* event);

	void eventMouseWheel(Event* event);

	void eventSize(Event* event);

	void eventPaint(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_PropertyList_H
