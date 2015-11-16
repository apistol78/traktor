#include <stack>
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Edit.h"
#include "Ui/HierarchicalState.h"
#include "Ui/Custom/TreeView/TreeView.h"
#include "Ui/Custom/TreeView/TreeViewContentChangeEvent.h"
#include "Ui/Custom/TreeView/TreeViewEditEvent.h"
#include "Ui/Custom/TreeView/TreeViewItem.h"

#include "Resources/TviState.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.TreeView", TreeView, AutoWidget)

TreeView::TreeView()
:	m_imageWidth(0)
,	m_imageHeight(0)
,	m_imageCount(0)
,	m_autoEdit(false)
{
}

bool TreeView::create(Widget* parent, int style)
{
	if (!AutoWidget::create(parent, style))
		return false;

	m_autoEdit = bool((style & WsAutoEdit) == WsAutoEdit);

	m_itemEditor = new Edit();
	m_itemEditor->create(this, L"", WsBorder);
	m_itemEditor->hide();
	m_itemEditor->addEventHandler< FocusEvent >(this, &TreeView::eventEditFocus);

	m_imageState = Bitmap::load(c_ResourceTviState, sizeof(c_ResourceTviState), L"png");

	addEventHandler< ScrollEvent >(this, &TreeView::eventScroll);
	return true;
}

int TreeView::addImage(Bitmap* image, int imageCount)
{
	if (m_image)
	{
		Ref< Bitmap > source = image;

		uint32_t width = m_image->getSize().cx + source->getSize().cx;
		uint32_t height = std::max(m_image->getSize().cy, source->getSize().cy);

		Ref< ui::Bitmap > newImage = new ui::Bitmap(width, height);
		newImage->copyImage(m_image->getImage());
		newImage->copySubImage(image->getImage(), Rect(Point(0, 0), source->getSize()), Point(m_image->getSize().cx, 0));
		m_image = newImage;
	}
	else
	{
		m_image = image;
		m_imageWidth = std::max< uint32_t >(m_imageWidth, m_image->getSize().cx / imageCount);
		m_imageHeight = std::max< uint32_t >(m_imageHeight, m_image->getSize().cy);
	}

	m_imageCount += imageCount;
	return m_imageCount - imageCount;
}

Ref< TreeViewItem > TreeView::createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage)
{
	Ref< TreeViewItem > item = new TreeViewItem(this, parent, text, image, expandedImage);

	if (parent)
		parent->m_children.push_back(item);
	else
		m_roots.push_back(item);

	requestUpdate();
	return item;
}

void TreeView::removeItem(TreeViewItem* item)
{
	if (item->m_parent)
	{
		T_ANONYMOUS_VAR(Ref< TreeViewItem >)(item);
		item->m_parent->m_children.remove(item);
	}
	else
		m_roots.remove(item);

	requestUpdate();
}

void TreeView::removeAllItems()
{
	m_roots.resize(0);
}

uint32_t TreeView::getItems(RefArray< TreeViewItem >& outItems, uint32_t flags) const
{
	typedef std::pair< RefArray< TreeViewItem >::const_iterator, RefArray< TreeViewItem >::const_iterator > range_t;

	std::stack< range_t > stack;
	stack.push(std::make_pair(m_roots.begin(), m_roots.end()));

	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			TreeViewItem* item = *r.first++;

			if (flags & GfSelectedOnly)
			{
				if (item->isSelected())
					outItems.push_back(item);
			}
			else
				outItems.push_back(item);

			if (flags & GfDescendants)
			{
				if ((flags & GfExpandedOnly) != GfExpandedOnly || item->isExpanded())
				{
					const RefArray< TreeViewItem >& children = item->getChildren();
					if (!children.empty())
						stack.push(std::make_pair(children.begin(), children.end()));
				}
			}
		}
		else
			stack.pop();
	}

	return uint32_t(outItems.size());
}

Ref< HierarchicalState > TreeView::captureState() const
{
	Ref< HierarchicalState > state = new HierarchicalState();

	RefArray< TreeViewItem > items;
	getItems(items, GfDescendants);
	for (RefArray< TreeViewItem >::iterator i = items.begin(); i != items.end(); ++i)
	{
		state->addState(
			(*i)->getPath(),
			(*i)->isExpanded(),
			(*i)->isSelected()
		);
	}

	return state;
}

void TreeView::applyState(const HierarchicalState* state)
{
	RefArray< TreeViewItem > items;
	getItems(items, GfDescendants);
	for (RefArray< TreeViewItem >::iterator i = items.begin(); i != items.end(); ++i)
	{
		std::wstring path = (*i)->getPath();

		if (state->getExpanded(path))
			(*i)->expand();
		else
			(*i)->collapse();

		if (state->getSelected(path))
			(*i)->select();
		else
			(*i)->unselect();
	}
}

void TreeView::layoutCells(const Rect& rc)
{
	int32_t height = getFont().getPixelSize() + scaleBySystemDPI(6);

	RefArray< TreeViewItem > items;
	getItems(items, GfDescendants | GfExpandedOnly);

	Rect rcRow(rc.left, rc.top, rc.right, rc.top + height);
	for (RefArray< TreeViewItem >::iterator i = items.begin(); i != items.end(); ++i)
	{
		placeCell(*i, rcRow);
		rcRow = rcRow.offset(0, height);
	}

	m_itemEditor->hide();
}

void TreeView::beginEdit(TreeViewItem* item)
{
	TreeViewEditEvent editEvent(this, item);
	raiseEvent(&editEvent);
	if (editEvent.consumed() && editEvent.cancelled())
		return;

	releaseCapturedCell();

	m_itemEditor->setRect(item->calculateLabelRect());
	m_itemEditor->setText(item->getText());
	m_itemEditor->selectAll();
	m_itemEditor->show();
	m_itemEditor->setFocus();

	m_editItem = item;
}

void TreeView::eventEditFocus(FocusEvent* event)
{
	if (event->lostFocus() && m_itemEditor->isVisible(false))
	{
		std::wstring originalText = m_editItem->getText();
		std::wstring newText = m_itemEditor->getText();

		m_itemEditor->hide();

		m_editItem->setText(newText);

		TreeViewContentChangeEvent changeEvent(this, m_editItem);
		raiseEvent(&changeEvent);

		if (!changeEvent.consumed())
			m_editItem->setText(originalText);
	}
}

void TreeView::eventScroll(ScrollEvent* event)
{
	m_itemEditor->hide();
}

		}
	}
}
