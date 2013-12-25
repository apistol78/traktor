#ifndef traktor_ui_GridView_H
#define traktor_ui_GridView_H

#include "Core/RefArray.h"
#include "Ui/Custom/Auto/AutoWidget.h"

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
		namespace custom
		{

class GridColumn;
class GridHeaderCell;
class GridRow;

/*! \brief Grid view control.
 * \ingroup UIC
 */
class T_DLLCLASS GridView : public AutoWidget
{
	T_RTTI_CLASS;

public:
	enum { EiExpand = EiUser + 1 };

	enum StyleFlags
	{
		WsColumnHeader = WsUser
	};

	enum GetFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfExpandedOnly = 2,
		GfSelectedOnly = 4
	};

	enum SortMode
	{
		SmLexical = 0,
		SmNumerical = 1
	};

	GridView();

	bool create(Widget* parent, uint32_t style);

	void addColumn(GridColumn* column);

	const RefArray< GridColumn >& getColumns() const;

	void setSortColumn(int32_t columnIndex, bool ascending, SortMode mode);

	int32_t getColumnIndex(int32_t x) const;

	void addRow(GridRow* row);

	void removeAllRows();

	const RefArray< GridRow >& getRows() const;

	uint32_t getRows(RefArray< GridRow >& outRows, uint32_t flags) const;

	GridRow* getSelectedRow() const;

	void addSelectEventHandler(EventHandler* eventHandler);

	void addClickEventHandler(EventHandler* eventHandler);

	void addExpandEventHandler(EventHandler* eventHandler);

private:
	Ref< GridHeaderCell > m_headerCell;
	RefArray< GridColumn > m_columns;
	RefArray< GridRow > m_rows;
	Ref< GridRow > m_clickRow;
	int32_t m_clickColumn;
	int32_t m_sortColumnIndex;
	bool m_sortAscending;
	SortMode m_sortMode;

	virtual void layoutCells(const Rect& rc);

	void eventButtonDown(Event* event);

	void eventButtonUp(Event* event);
};

		}
	}
}

#endif	// traktor_ui_GridView_H
