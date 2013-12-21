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
	// @fixme Not safe, we must check so the internal handle in fact are a Gtk::Fixed container. Also need to ensure cleanup of m_parentContainer.
	m_parentContainer = static_cast< Gtk::Fixed* >(parent->getInternalHandle());
	if (!m_parentContainer)
	{
		log::error << L"Unable to get parent GTK container; must be a GTK fixed container" << Endl;
		return false;
	}

	Gtk::TreeView* treeView = new Gtk::TreeView();
	//button->signal_clicked().connect(sigc::mem_fun(*this, &TreeViewGtk::on_button_clicked));

	m_parentContainer->put(*treeView, 0, 0);

	treeView->show();

	m_widget = treeView;

	return true;
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

