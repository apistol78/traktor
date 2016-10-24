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

class Edit;

		namespace custom
		{

class GridItem;
class GridColumn;
class GridHeader;
class GridRow;

/*! \brief Grid view control.
 * \ingroup UIC
 */
class T_DLLCLASS GridView : public AutoWidget
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsColumnHeader = WsUser,
		WsAutoEdit = WsUser << 1,
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

	GridColumn* getColumn(uint32_t index) const;

	const RefArray< GridColumn >& getColumns() const;

	void setSortColumn(int32_t columnIndex, bool ascending, SortMode mode);

	int32_t getColumnIndex(int32_t x) const;

	void addRow(GridRow* row);

	void removeRow(GridRow* row);

	void removeAllRows();

	GridRow* getRow(int32_t index);

	const RefArray< GridRow >& getRows() const;

	uint32_t getRows(RefArray< GridRow >& outRows, uint32_t flags) const;

	GridRow* getSelectedRow() const;

	void selectAll();

	void deselectAll();

private:
	friend class GridItem;
	friend class GridRow;

	Ref< GridHeader > m_header;
	RefArray< GridColumn > m_columns;
	RefArray< GridRow > m_rows;
	Ref< GridRow > m_clickRow;
	int32_t m_clickColumn;
	int32_t m_sortColumnIndex;
	bool m_sortAscending;
	SortMode m_sortMode;
	bool m_autoEdit;
	Ref< Edit > m_itemEditor;
	Ref< GridItem > m_editItem;

	virtual void layoutCells(const Rect& rc) T_OVERRIDE T_FINAL;

	void beginEdit(GridItem* item);

	void eventEditFocus(FocusEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);
};

		}
	}
}

#endif	// traktor_ui_GridView_H
