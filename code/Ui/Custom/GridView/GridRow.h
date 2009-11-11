#ifndef traktor_ui_GridRow_H
#define traktor_ui_GridRow_H

#include "Core/Object.h"
#include "Core/Heap/Ref.h"
#include "Core/Heap/RefArray.h"
#include "Ui/Associative.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Font;

		namespace custom
		{

class GridItem;

/*! \brief Grid row.
 * \ingroup UIC
 */
class T_DLLCLASS GridRow
:	public Object
,	public Associative
{
	T_RTTI_CLASS(GridRow)

public:
	enum RowStates
	{
		RsExpanded = 1,
		RsSelected = 2
	};

	GridRow(uint32_t initialState = RsExpanded);

	void setState(uint32_t state);

	uint32_t getState() const { return m_state; }

	void setFont(Font* font);

	Ref< Font > getFont() const { return m_font; }

	void addItem(GridItem* item);

	const RefArray< GridItem >& getItems() const { return m_items; }

	void addChild(GridRow* row);

	void insertChildBefore(GridRow* insertBefore, GridRow* row);

	void insertChildAfter(GridRow* insertAfter, GridRow* row);

	void removeChild(GridRow* row);

	void removeAllChildren();

	Ref< GridRow > getParent() { return m_parent; }

	const RefArray< GridRow >& getChildren() const { return m_children; }

private:
	uint32_t m_state;
	Ref< Font > m_font;
	RefArray< GridItem > m_items;
	Ref< GridRow > m_parent;
	RefArray< GridRow > m_children;
};

		}
	}
}

#endif	// traktor_ui_GridRow_H
