#ifndef traktor_ui_GridView_H
#define traktor_ui_GridView_H

#include "Core/Heap/Ref.h"
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
	T_RTTI_CLASS(GridView)

public:
	enum StyleFlags
	{
		WsColumnHeader = WsUser,
		WsDrag = (WsUser << 1)
	};

	enum GetFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfExpandedOnly = 2,
		GfSelectedOnly = 4
	};

	enum Events
	{
		EiDrag = EiUser + 1,
		EiDragValid = EiUser + 2
	};

	GridView();

	bool create(Widget* parent, uint32_t style);

	uint32_t addImage(Bitmap* image, uint32_t imageCount);

	void addColumn(GridColumn* column);

	const RefArray< GridColumn >& getColumns() const { return m_columns; }

	void addRow(GridRow* row);

	void removeAllRows();

	uint32_t getRows(RefArray< GridRow >& outRows, uint32_t flags) const;

	void addSelectEventHandler(EventHandler* eventHandler);

	void addDragEventHandler(EventHandler* eventHandler);

	void addDragValidEventHandler(EventHandler* eventHandler);

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
	int m_resizeColumnLeft;
	bool m_dragEnabled;
	Ref< GridRow > m_dragRow;
	bool m_dragActive;
	Point m_dragPosition;
	bool m_columnHeader;
	int m_lastSelected;

	void updateScrollBars();

	int dropRow() const;

	bool dropValid();

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
