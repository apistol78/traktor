#ifndef traktor_ui_TreeView_H
#define traktor_ui_TreeView_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class HierarchicalState;
class TreeViewItem;
		
/*! \brief Tree view.
 * \ingroup UI
 */
class T_DLLCLASS TreeView : public Widget
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

	bool create(Widget* parent, int style = WsDefault);

	int addImage(Bitmap* image, int imageCount);

	Ref< TreeViewItem > createItem(TreeViewItem* parent, const std::wstring& text, int image = -1, int expandedImage = -1);

	void removeItem(TreeViewItem* item);

	void removeAllItems();

	Ref< TreeViewItem > getRootItem() const;

	Ref< TreeViewItem > getSelectedItem() const;

	Ref< HierarchicalState > captureState() const;

	void applyState(const HierarchicalState* state);
};

	}
}

#endif	// traktor_ui_TreeView_H
