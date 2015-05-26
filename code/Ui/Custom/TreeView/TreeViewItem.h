#ifndef traktor_ui_custom_TreeViewItem_H
#define traktor_ui_custom_TreeViewItem_H

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
		namespace custom
		{

class TreeView;

/*! \brief Tree view item.
 * \ingroup UIC
 */
class T_DLLCLASS TreeViewItem : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	TreeViewItem(TreeView* view, TreeViewItem* parent, const std::wstring& text, int32_t image, int32_t expandedImage);

	void setText(const std::wstring& text);

	std::wstring getText() const;

	void setBold(bool bold);

	bool isBold() const;

	void setImage(int32_t image);

	int32_t getImage() const;

	void setExpandedImage(int32_t expandedImage);

	int32_t getExpandedImage() const;

	bool isExpanded() const;

	void expand();

	bool isCollapsed() const;

	void collapse();

	bool isSelected() const;

	void select();

	void unselect();

	bool isVisible() const;

	void show();

	void setEditable(bool editable);

	bool isEditable() const;

	bool edit();

	void sort(bool recursive);

	Ref< TreeViewItem > getParent() const;

	bool hasChildren() const;

	const RefArray< TreeViewItem >& getChildren() const;

	Ref< TreeViewItem > findChild(const std::wstring& childPath);

	/*! \brief Get path to this item.
	 *
	 * Path is separated with / for each level.
	 */
	std::wstring getPath() const;

private:
	friend class TreeView;

	TreeView* m_view;
	TreeViewItem* m_parent;
	std::wstring m_text;
	int32_t m_image;
	int32_t m_expandedImage;
	bool m_expanded;
	bool m_selected;
	bool m_editable;
	int32_t m_editMode;
	int32_t m_dragMode;
	RefArray< TreeViewItem > m_children;

	int32_t calculateDepth() const;

	Rect calculateExpandRect() const;

	Rect calculateImageRect() const;

	Rect calculateLabelRect() const;

	virtual void interval() T_FINAL;

	virtual void mouseDown(MouseButtonDownEvent* event, const Point& position) T_FINAL;

	virtual void mouseUp(MouseButtonUpEvent* event, const Point& position) T_FINAL;

	virtual void mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position) T_FINAL;

	virtual void mouseMove(MouseMoveEvent* event, const Point& position) T_FINAL;

	virtual void paint(Canvas& canvas, const Rect& rect) T_FINAL;
};

		}
	}
}

#endif	// traktor_ui_custom_TreeViewItem_H
