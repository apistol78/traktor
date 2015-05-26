#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/TreeView/TreeView.h"
#include "Ui/Custom/TreeView/TreeViewDragEvent.h"
#include "Ui/Custom/TreeView/TreeViewItem.h"
#include "Ui/Custom/TreeView/TreeViewItemActivateEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.TreeViewItem", TreeViewItem, Object)

TreeViewItem::TreeViewItem(TreeView* view, TreeViewItem* parent, const std::wstring& text, int32_t image, int32_t expandedImage)
:	m_view(view)
,	m_parent(parent)
,	m_text(text)
,	m_image(image)
,	m_expandedImage(expandedImage)
,	m_expanded(false)
,	m_selected(false)
,	m_editable(false)
,	m_editMode(0)
,	m_dragMode(0)
{
}

void TreeViewItem::setText(const std::wstring& text)
{
	m_text = text;
}

std::wstring TreeViewItem::getText() const
{
	return m_text;
}

void TreeViewItem::setBold(bool bold)
{
}

bool TreeViewItem::isBold() const
{
	return false;
}

void TreeViewItem::setImage(int32_t image)
{
	m_image = image;
}

int32_t TreeViewItem::getImage() const
{
	return m_image;
}

void TreeViewItem::setExpandedImage(int32_t expandedImage)
{
	m_expandedImage = expandedImage;
}

int32_t TreeViewItem::getExpandedImage() const
{
	return m_expandedImage;
}

bool TreeViewItem::isExpanded() const
{
	return m_expanded;
}

void TreeViewItem::expand()
{
	m_expanded = true;
}

bool TreeViewItem::isCollapsed() const
{
	return !m_expanded;
}

void TreeViewItem::collapse()
{
	m_expanded = false;
}

bool TreeViewItem::isSelected() const
{
	return m_selected;
}

void TreeViewItem::select()
{
	m_selected = true;
}

void TreeViewItem::unselect()
{
	m_selected = false;
}

bool TreeViewItem::isVisible() const
{
	return true;
}

void TreeViewItem::show()
{
	for (TreeViewItem* parent = m_parent; parent; parent = parent->m_parent)
		parent->expand();
}

void TreeViewItem::setEditable(bool editable)
{
	m_editable = editable;
}

bool TreeViewItem::isEditable() const
{
	return m_editable;
}

bool TreeViewItem::edit()
{
	if (m_editable)
	{
		m_editMode = 0;
		m_view->beginEdit(this);
		return true;
	}
	else
		return false;
}

void TreeViewItem::sort(bool recursive)
{
}

Ref< TreeViewItem > TreeViewItem::getParent() const
{
	return m_parent;
}

bool TreeViewItem::hasChildren() const
{
	return !m_children.empty();
}

const RefArray< TreeViewItem >& TreeViewItem::getChildren() const
{
	return m_children;
}

Ref< TreeViewItem > TreeViewItem::findChild(const std::wstring& childPath)
{
	std::vector< std::wstring > childNames;
	if (Split< std::wstring >::any(childPath, L"/", childNames) <= 0)
		return 0;

	Ref< TreeViewItem > item = this;
	for (std::vector< std::wstring >::iterator i = childNames.begin(); i != childNames.end(); ++i)
	{
		const RefArray< TreeViewItem >& children = item->m_children;

		Ref< TreeViewItem > next;
		for (RefArray< TreeViewItem >::const_iterator j = children.begin(); j != children.end(); ++j)
		{
			if ((*j)->getText() == *i)
			{
				next = *j;
				break;
			}
		}

		if (!(item = next))
			return 0;
	}

	return item;
}

std::wstring TreeViewItem::getPath() const
{
	return getParent() ? getParent()->getPath() + L'/' + getText() : L"";
}

int32_t TreeViewItem::calculateDepth() const
{
	int32_t depth = 0;
	for (const TreeViewItem* item = m_parent; item; item = item->m_parent)
		++depth;
	return depth;
}

Rect TreeViewItem::calculateExpandRect() const
{
	int32_t depth = calculateDepth();

	Rect rcItem = m_view->getCellClientRect(this);
	rcItem.left += 4 + depth * 20;
	rcItem.right = rcItem.left + 16;

	int32_t dy = (rcItem.getHeight() - 16) / 2;
	rcItem.top += dy;
	rcItem.bottom = rcItem.top + 16;

	return rcItem;
}

Rect TreeViewItem::calculateImageRect() const
{
	int32_t depth = calculateDepth();

	Rect rcItem = m_view->getCellClientRect(this);
	rcItem.left += 4 + depth * 20 + 22;
	rcItem.right = rcItem.left + 16;

	int32_t dy = (rcItem.getHeight() - 16) / 2;
	rcItem.top += dy;
	rcItem.bottom = rcItem.top + 16;

	return rcItem;
}

Rect TreeViewItem::calculateLabelRect() const
{
	Size extent = m_view->getTextExtent(m_text);
	int32_t depth = calculateDepth();

	Rect rcItem = m_view->getCellClientRect(this);
	rcItem.left += 4 + depth * 20 + 44;
	rcItem.right = rcItem.left + extent.cx + 16;

	return rcItem;
}

void TreeViewItem::interval()
{
	// Cancel pending edit.
	m_editMode = 0;
}

void TreeViewItem::mouseDown(MouseButtonDownEvent* event, const Point& position)
{
	if (hasChildren() && calculateExpandRect().inside(event->getPosition()))
	{
		m_expanded = !m_expanded;
	}
	else
	{
		if (!isSelected())
		{
			// De-select all items.
			RefArray< TreeViewItem > selectedItems;
			m_view->getItems(selectedItems, TreeView::GfDescendants | TreeView::GfSelectedOnly);
			for (RefArray< TreeViewItem >::iterator i = selectedItems.begin(); i != selectedItems.end(); ++i)
				(*i)->unselect();

			// Select this item only.
			select();

			SelectionChangeEvent selectionChangeEvent(m_view);
			m_view->raiseEvent(&selectionChangeEvent);
		}

		if (calculateLabelRect().inside(event->getPosition()))
		{
			if (m_editable)
			{
				if (m_editMode == 0)
				{
					// Wait for next tap; cancel wait after 2 seconds.
					m_view->requestInterval(this, 2000);
					m_editMode = 1;
				}
				else if (m_editMode == 1)
				{
					// Double tap detected; begin edit after mouse is released.
					m_editMode = 2;
				}
			}

			m_dragMode = 1;
		}
	}

	m_view->requestUpdate();
}

void TreeViewItem::mouseUp(MouseButtonUpEvent* event, const Point& position)
{
	if (m_editMode == 2)
	{
		T_ASSERT (m_editable);
		m_view->beginEdit(this);
	}

	if (m_dragMode == 2)
	{
		Point position = m_view->clientToScreen(event->getPosition());
		TreeViewDragEvent dragEvent(m_view, this, TreeViewDragEvent::DmDrop, position);
		m_view->raiseEvent(&dragEvent);
	}

	m_editMode = 0;
	m_dragMode = 0;
}

void TreeViewItem::mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position)
{
	// Ensure edit isn't triggered.
	m_editMode = 0;

	// Raise activation event.
	TreeViewItemActivateEvent activateEvent(m_view, this);
	m_view->raiseEvent(&activateEvent);
}

void TreeViewItem::mouseMove(MouseMoveEvent* event, const Point& position)
{
	if (m_dragMode == 1)
	{
		TreeViewDragEvent dragEvent(m_view, this, TreeViewDragEvent::DmDrag);
		m_view->raiseEvent(&dragEvent);
		if (!(dragEvent.consumed() && dragEvent.cancelled()))
			m_dragMode = 2;
		else
			m_dragMode = 3;
	}
}

void TreeViewItem::paint(Canvas& canvas, const Rect& rect)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	if (isSelected())
	{
		canvas.setBackground(ss->getColor(m_view, L"item-background-color-selected"));
		canvas.fillRect(rect);
	}

	if (m_view->m_imageState && hasChildren())
	{
		Rect rcExpand = calculateExpandRect();
		canvas.drawBitmap(
			rcExpand.getTopLeft(),
			Point(isExpanded() ? 16 : 0, isSelected() ? 16 : 0),
			Size(16, 16),
			m_view->m_imageState,
			BmAlpha
		);
	}

	int32_t image = (hasChildren() && isExpanded()) ? m_expandedImage : m_image;
	if (m_view->m_image && image >= 0)
	{
		Rect rcImage = calculateImageRect();
		canvas.drawBitmap(
			rcImage.getTopLeft(),
			Point(image * 16, 0),
			Size(16, 16),
			m_view->m_image,
			BmAlpha
		);
	}

	if (!m_text.empty())
	{
		Rect rcLabel = calculateLabelRect();
		canvas.setForeground(ss->getColor(m_view, m_selected ? L"item-color-selected" : L"color"));
		canvas.drawText(rcLabel, m_text, AnLeft, AnCenter);
	}
}

		}
	}
}
