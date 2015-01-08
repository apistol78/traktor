#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Gtk/TreeViewGtk.h"

namespace traktor
{
	namespace ui
	{

TreeViewGtk::TreeViewGtk(EventSubject* owner)
:	WidgetGtkImpl< ITreeView >(owner)
{
}

bool TreeViewGtk::create(IWidget* parent, int style)
{
	Internal* parentInternal = static_cast< Internal* >(parent->getInternalHandle());
	T_FATAL_ASSERT(parentInternal);

	Gtk::Fixed* container = new Gtk::Fixed();
	parentInternal->container->put(*container, 0, 0);

	Gtk::TreeView* treeView = new Gtk::TreeView();
	container->put(*treeView, 0, 0);
	treeView->show();

	m_internal.container = container;
	m_internal.widget = treeView;

	return WidgetGtkImpl< ITreeView >::create();
}

int TreeViewGtk::addImage(IBitmap* image, int imageCount)
{
	return 0;
}

Ref< TreeViewItem > TreeViewGtk::createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage)
{
	return 0;
}

void TreeViewGtk::removeItem(TreeViewItem* item)
{
}

void TreeViewGtk::removeAllItems()
{
}

Ref< TreeViewItem > TreeViewGtk::getRootItem() const
{
	return 0;
}

Ref< TreeViewItem > TreeViewGtk::getSelectedItem() const
{
	return 0;
}

	}
}

