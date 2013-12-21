#ifndef traktor_ui_TreeViewGtk_H
#define traktor_ui_TreeViewGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/ITreeView.h"

namespace traktor
{
	namespace ui
	{

class TreeViewGtk : public WidgetGtkImpl< ITreeView >
{
public:
	TreeViewGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual int addImage(IBitmap* image, int imageCount);

	virtual Ref< TreeViewItem > createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage);

	virtual void removeItem(TreeViewItem* item);

	virtual void removeAllItems();

	virtual Ref< TreeViewItem > getRootItem() const;

	virtual Ref< TreeViewItem > getSelectedItem() const;
};

	}
}

#endif	// traktor_ui_TreeViewGtk_H

