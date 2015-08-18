#ifndef traktor_ui_TreeView_H
#define traktor_ui_TreeView_H

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
class HierarchicalState;

		namespace custom
		{

class TreeViewItem;

/*! \brief Tree view control.
 * \ingroup UIC
 */
class T_DLLCLASS TreeView : public AutoWidget
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsAutoEdit = WsUser,
		WsDrag = WsUser << 1,
		WsTreeButtons = WsUser << 2,
		WsTreeLines = WsUser << 3,
		WsDefault = WsClientBorder | WsAutoEdit | WsTreeButtons | WsTreeLines
	};

	enum GetFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfExpandedOnly = 2,
		GfSelectedOnly = 4
	};

	TreeView();

	bool create(Widget* parent, int style = WsDefault);

	int addImage(Bitmap* image, int imageCount);

	Ref< TreeViewItem > createItem(TreeViewItem* parent, const std::wstring& text, int image = -1, int expandedImage = -1);

	void removeItem(TreeViewItem* item);

	void removeAllItems();

	uint32_t getItems(RefArray< TreeViewItem >& outItems, uint32_t flags) const;

	Ref< HierarchicalState > captureState() const;

	void applyState(const HierarchicalState* state);

private:
	friend class TreeViewItem;

	RefArray< TreeViewItem > m_roots;
	Ref< Bitmap > m_imageState;
	Ref< Bitmap > m_image;
	int32_t m_imageWidth;
	int32_t m_imageHeight;
	int32_t m_imageCount;
	Ref< Edit > m_itemEditor;
	Ref< TreeViewItem > m_editItem;
	bool m_autoEdit;

	virtual void layoutCells(const Rect& rc);

	void beginEdit(TreeViewItem* item);

	void eventEditFocus(FocusEvent* event);
};

		}
	}
}

#endif	// traktor_ui_TreeView_H
