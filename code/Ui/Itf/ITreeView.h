#ifndef traktor_ui_ITreeView_H
#define traktor_ui_ITreeView_H

#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

class ISystemBitmap;
class TreeViewItem;

/*! \brief TreeView interface.
 * \ingroup UI
 */
class ITreeView : public IWidget
{
public:
	virtual bool create(IWidget* parent, int style) = 0;

	virtual int addImage(ISystemBitmap* image, int imageCount) = 0;

	virtual Ref< TreeViewItem > createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage) = 0;

	virtual void removeItem(TreeViewItem* item) = 0;

	virtual void removeAllItems() = 0;

	virtual Ref< TreeViewItem > getRootItem() const = 0;

	virtual Ref< TreeViewItem > getSelectedItem() const = 0;
};

	}
}

#endif	// traktor_ui_ITreeView_H
