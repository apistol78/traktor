#ifndef traktor_ui_GridRow_H
#define traktor_ui_GridRow_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

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

class IBitmap;

		namespace custom
		{

class GridItem;

/*! \brief Grid row.
 * \ingroup UIC
 */
class T_DLLCLASS GridRow : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	enum RowStates
	{
		RsExpanded = 1,
		RsSelected = 2
	};

	GridRow(uint32_t initialState = RsExpanded);

	virtual ~GridRow();

	void setState(uint32_t state);

	uint32_t getState() const { return m_state; }

	void setBackground(const Color4ub& background);

	const Color4ub& getBackground() const { return m_background; }

	void setMinimumHeight(int32_t minimumHeight);

	const int32_t getMinimumHeight() const { return m_minimumHeight; }

	int32_t getHeight() const;

	uint32_t add(GridItem* item);

	void set(uint32_t index, GridItem* item);

	Ref< GridItem > get(uint32_t index) const;

	const RefArray< GridItem >& get() const { return m_items; }

	void addChild(GridRow* row);

	void insertChildBefore(GridRow* insertBefore, GridRow* row);

	void insertChildAfter(GridRow* insertAfter, GridRow* row);

	void removeChild(GridRow* row);

	void removeAllChildren();

	GridRow* getParent() { return m_parent; }

	const RefArray< GridRow >& getChildren() const { return m_children; }

	virtual void placeCells(AutoWidget* widget, const Rect& rect) T_OVERRIDE T_FINAL;

	virtual void mouseDown(MouseButtonDownEvent* event, const Point& position) T_OVERRIDE T_FINAL;

	virtual void paint(Canvas& canvas, const Rect& rect) T_OVERRIDE T_FINAL;

private:
	friend class GridView;

	uint32_t m_state;
	Color4ub m_background;
	int32_t m_minimumHeight;
	Ref< IBitmap > m_expand;
	RefArray< GridItem > m_items;
	GridRow* m_parent;
	RefArray< GridRow > m_children;

	int32_t getDepth() const;
};

		}
	}
}

#endif	// traktor_ui_GridRow_H
