#ifndef traktor_ui_GridView_H
#define traktor_ui_GridView_H

#include "Core/RefArray.h"
#include "Ui/Widget.h"

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

class Bitmap;
class ScrollBar;

		namespace custom
		{

class GridColumn;
class GridRow;

/*! \brief Grid view control.
 * \ingroup UIC
 */
class T_DLLCLASS GridView : public Widget
{
	T_RTTI_CLASS;

public:
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

	GridView();

	bool create(Widget* parent, uint32_t style);

	uint32_t addImage(Bitmap* image, uint32_t imageCount);

	void addColumn(GridColumn* column);

	const RefArray< GridColumn >& getColumns() const { return m_columns; }

	void addRow(GridRow* row);

	void removeAllRows();

	const RefArray< GridRow >& getRows() const;

	uint32_t getRows(RefArray< GridRow >& outRows, uint32_t flags) const;

	GridRow* getSelectedRow() const;

	void addSelectEventHandler(EventHandler* eventHandler);

	void addClickEventHandler(EventHandler* eventHandler);

	virtual Size getPreferedSize() const;

	virtual void update(const Rect* rc = 0, bool immediate = false);

private:
	Ref< Bitmap > m_image;
	uint32_t m_imageWidth;
	uint32_t m_imageHeight;
	uint32_t m_imageCount;
	RefArray< GridColumn > m_columns;
	RefArray< GridRow > m_rows;
	Ref< Bitmap > m_expand[2];
	Ref< ScrollBar > m_scrollBarRows;
	Ref< GridColumn > m_resizeColumn;
	int32_t m_resizeColumnLeft;
	bool m_columnHeader;
	int32_t m_lastSelected;

	Ref< GridRow > m_clickRow;
	int32_t m_clickColumn;

	void updateScrollBars();

	void getColumnPositions(std::vector< int32_t >& outColumnPos) const;

	void getColumnSplits(std::vector< int32_t >& outColumnSplits) const;

	int32_t getColumnIndex(int32_t position) const;

	void eventButtonDown(Event* event);

	void eventButtonUp(Event* event);

	void eventMouseMove(Event* event);

	void eventMouseWheel(Event* event);

	void eventPaint(Event* event);

	void eventSize(Event* event);

	void eventScroll(Event* event);
};

		}
	}
}

#endif	// traktor_ui_GridView_H
