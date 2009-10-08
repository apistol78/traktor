#ifndef traktor_ui_TreeViewCocoa_H
#define traktor_ui_TreeViewCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/ITreeView.h"

namespace traktor
{
	namespace ui
	{

class TreeViewCocoa : public WidgetCocoaImpl< ITreeView, NSControl >
{
public:
	TreeViewCocoa(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual int addImage(IBitmap* image, int imageCount);

	virtual TreeViewItem* createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage);

	virtual void removeItem(TreeViewItem* item);

	virtual void removeAllItems();

	virtual TreeViewItem* getRootItem() const;

	virtual TreeViewItem* getSelectedItem() const;
};

	}
}

#endif	// traktor_ui_TreeViewCocoa_H
