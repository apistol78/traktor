#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/HierarchicalState.h"
#include "Ui/TreeView.h"
#include "Ui/TreeViewItem.h"
#include "Ui/Itf/ITreeView.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

void recursiveCaptureState(TreeViewItem* item, HierarchicalState* outState)
{
	if (!item)
		return;

	std::wstring path = item->getPath();
	outState->addState(path, item->isExpanded(), item->isSelected());

	RefArray< TreeViewItem > children;
	item->getChildren(children);

	for (RefArray< TreeViewItem >::iterator i = children.begin(); i != children.end(); ++i)
		recursiveCaptureState(*i, outState);
}

void recursiveApplyState(TreeViewItem* item, const HierarchicalState* state)
{
	if (!item)
		return;

	RefArray< TreeViewItem > children;
	item->getChildren(children);

	for (RefArray< TreeViewItem >::iterator i = children.begin(); i != children.end(); ++i)
		recursiveApplyState(*i, state);

	std::wstring path = item->getPath();
	
	if (state->getExpanded(path))
		item->expand();
	else
		item->collapse();

	if (state->getSelected(path))
		item->select();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeView", TreeView, Widget)

bool TreeView::create(Widget* parent, int style)
{
	if (!parent)
		return false;

	ITreeView* treeView = Application::getInstance()->getWidgetFactory()->createTreeView(this);
	if (!treeView)
	{
		log::error << L"Failed to create native widget peer (TreeView)" << Endl;
		return false;
	}

	if (!treeView->create(parent->getIWidget(), style))
	{
		treeView->destroy();
		return false;
	}

	m_widget = treeView;

	return Widget::create(parent, style);
}

int TreeView::addImage(Bitmap* image, int imageCount)
{
	if (!image || !image->getIBitmap())
		return -1;
	return static_cast< ITreeView* >(m_widget)->addImage(image->getIBitmap(), imageCount);
}

Ref< TreeViewItem > TreeView::createItem(TreeViewItem* parent, const std::wstring& text, int image, int selectedImage)
{
	return static_cast< ITreeView* >(m_widget)->createItem(parent, text, image, selectedImage);
}

void TreeView::removeItem(TreeViewItem* item)
{
	T_ASSERT (item);
	static_cast< ITreeView* >(m_widget)->removeItem(item);
}

void TreeView::removeAllItems()
{
	static_cast< ITreeView* >(m_widget)->removeAllItems();
}

Ref< TreeViewItem > TreeView::getRootItem() const
{
	return static_cast< ITreeView* >(m_widget)->getRootItem();
}

Ref< TreeViewItem > TreeView::getSelectedItem() const
{
	return static_cast< ITreeView* >(m_widget)->getSelectedItem();
}

Ref< HierarchicalState > TreeView::captureState() const
{
	Ref< HierarchicalState > state = new HierarchicalState();
	recursiveCaptureState(getRootItem(), state);
	return state;
}

void TreeView::applyState(const HierarchicalState* state)
{
	// Temporarily hide tree to prevent multiple updates being issued when
	// applying states.
	bool visible = isVisible(false);
	setVisible(false);

	recursiveApplyState(getRootItem(), state);

	setVisible(visible);
}

	}
}
