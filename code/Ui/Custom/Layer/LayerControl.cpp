#include <algorithm>
#include <stack>
#include "Drawing/Image.h"
#include "Ui/Bitmap.h"
#include "Ui/ScrollBar.h"
#include "Ui/Custom/Layer/LayerContentChangeEvent.h"
#include "Ui/Custom/Layer/LayerControl.h"
#include "Ui/Custom/Layer/LayerItem.h"

// Resources
#include "Resources/LayerVisible.h"
#include "Resources/LayerHidden.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_layerItemEnableWidth = 36;
const int c_layerItemHeight = 24;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.LayerControl", LayerControl, Widget)

bool LayerControl::create(Widget* parent, int style)
{
	if (!Widget::create(parent, style))
		return false;

	m_scrollBar = new ScrollBar();
	if (!m_scrollBar->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBar->addEventHandler< ScrollEvent >(this, &LayerControl::eventScroll);

	m_imageVisible = Bitmap::load(c_ResourceLayerVisible, sizeof(c_ResourceLayerVisible), L"image");
	m_imageHidden = Bitmap::load(c_ResourceLayerHidden, sizeof(c_ResourceLayerHidden), L"image");

	addEventHandler< SizeEvent >(this, &LayerControl::eventSize);
	addEventHandler< MouseButtonDownEvent >(this, &LayerControl::eventButtonDown);
	addEventHandler< PaintEvent >(this, &LayerControl::eventPaint);

	return true;
}

void LayerControl::addLayerItem(LayerItem* layerItem)
{
	T_ASSERT (!layerItem->getParentLayer());
	m_layers.push_back(layerItem);
	
	updateScrollBar();
}

void LayerControl::removeLayerItem(LayerItem* layerItem)
{
	if (!layerItem->getParentLayer())
	{
		RefArray< LayerItem >::iterator i = std::find(m_layers.begin(), m_layers.end(), layerItem);
		T_ASSERT (i != m_layers.end());
		m_layers.erase(i);
	}
	else
		layerItem->getParentLayer()->removeChildLayer(layerItem);
	
	updateScrollBar();
}

void LayerControl::removeAllLayerItems()
{
	m_layers.clear();
	updateScrollBar();
}

RefArray< LayerItem >& LayerControl::getLayerItems()
{
	return m_layers;
}

int LayerControl::getItems(RefArray< LayerItem >& outItems, int flags)
{
	typedef std::pair< RefArray< LayerItem >::iterator, RefArray< LayerItem >::iterator > range_t;

	std::stack< range_t > stack;
	stack.push(std::make_pair(m_layers.begin(), m_layers.end()));

	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			LayerItem* item = *r.first++;

			if (flags & GfSelectedOnly)
			{
				if (item->isSelected())
					outItems.push_back(item);
			}
			else
				outItems.push_back(item);

			if (flags & GfDescendants)
			{
				RefArray< LayerItem >& childLayers = item->getChildLayers();
				if (!childLayers.empty())
					stack.push(std::make_pair(childLayers.begin(), childLayers.end()));
			}
		}
		else
			stack.pop();
	}

	return int(outItems.size());
}

Ref< LayerItem > LayerControl::getLayerItem(int index, bool includeChildren)
{
	typedef std::pair< RefArray< LayerItem >::iterator, RefArray< LayerItem >::iterator > range_t;
	
	std::stack< range_t > stack;
	stack.push(std::make_pair(m_layers.begin(), m_layers.end()));

	while (!stack.empty())
	{
		range_t& r = stack.top();
		if (r.first != r.second)
		{
			LayerItem* item = *r.first++;
			if (!index--)
				return item;

			RefArray< LayerItem >& childLayers = item->getChildLayers();
			if (!childLayers.empty())
				stack.push(std::make_pair(childLayers.begin(), childLayers.end()));
		}
		else
			stack.pop();
	}

	return 0;
}

Size LayerControl::getPreferedSize() const
{
	return Size(256, 256);
}

void LayerControl::updateScrollBar()
{
	Rect rc = getInnerRect();

	// Traverse all items, including children.
	RefArray< LayerItem > items;
	getItems(items, GfDefault | GfDescendants);

	// Calculate height of all items and see how much it's goes beyond limits.
	int layerHeight = int(items.size()) * c_layerItemHeight - 1;
	int overflow = std::max< int >(0, layerHeight - rc.getHeight());

	m_scrollBar->setRange(overflow);
	m_scrollBar->setEnable(overflow > 0);
}

void LayerControl::paintItem(Canvas& canvas, Rect& rcItem, LayerItem* item, int childLevel)
{
	if (!item->isSelected())
	{
		canvas.setForeground(Color4ub(250, 249, 250));
		canvas.setBackground(Color4ub(238, 237, 240));
		canvas.fillGradientRect(rcItem);
	}
	else
	{
		canvas.setBackground(Color4ub(226, 229, 238));
		canvas.fillRect(rcItem);
	}

	canvas.setForeground(Color4ub(128, 128, 128));
	canvas.drawLine(rcItem.left + c_layerItemEnableWidth, rcItem.top, rcItem.left + c_layerItemEnableWidth, rcItem.bottom - 1);
	canvas.drawLine(rcItem.left, rcItem.bottom - 1, rcItem.right, rcItem.bottom - 1);

	if (childLevel)
	{
		int x = rcItem.left + c_layerItemEnableWidth + 4 + (childLevel - 1) * 16 + 4;
		canvas.drawLine(x, rcItem.top + 4, x, rcItem.top + rcItem.getHeight() / 2);
		canvas.drawLine(x, rcItem.top + rcItem.getHeight() / 2, x + 8, rcItem.top + rcItem.getHeight() / 2);
	}

	Size ext = canvas.getTextExtent(item->getText());

	canvas.setForeground(Color4ub(0, 0, 0));
	canvas.drawText(
		Point(
			rcItem.left + c_layerItemEnableWidth + 4 + childLevel * 16,
			rcItem.top + (rcItem.getHeight() - ext.cy) / 2
		),
		item->getText()
	);

	Ref< Bitmap > image = item->isEnabled() ? m_imageVisible : m_imageHidden;
	if (image)
	{
		Point pt(
			rcItem.left + (c_layerItemEnableWidth - image->getSize().cx) / 2,
			rcItem.top + (rcItem.getHeight() - image->getSize().cy) / 2
		);
		canvas.drawBitmap(
			pt,
			Point(0, 0),
			image->getSize(),
			image
		);
	}

	rcItem = rcItem.offset(0, c_layerItemHeight);

	RefArray< LayerItem >& childLayers = item->getChildLayers();
	for (RefArray< LayerItem >::iterator i = childLayers.begin(); i != childLayers.end(); ++i)
		paintItem(canvas, rcItem, *i, childLevel + 1);
}

void LayerControl::eventScroll(ScrollEvent* event)
{
	update();
	event->consume();
}

void LayerControl::eventSize(SizeEvent* event)
{
	event->consume();

	Rect rc = getInnerRect();

	int scrollWidth = m_scrollBar->getPreferedSize().cx;

	m_scrollBar->setRect(Rect(
		rc.right - scrollWidth,
		rc.top,
		rc.right,
		rc.bottom
	));

	updateScrollBar();
}

void LayerControl::eventButtonDown(MouseButtonDownEvent* event)
{
	Point pt = event->getPosition();

	int scrollOffset = m_scrollBar->getPosition();
	int id = (pt.y + scrollOffset) / c_layerItemHeight;

	// Toggle selection on pressed item.
	Ref< LayerItem > layerItem = getLayerItem(id);
	if (layerItem)
	{
		if (pt.x < c_layerItemEnableWidth)
		{
			layerItem->setSelected(true);
			layerItem->setEnable(!layerItem->isEnabled());
			
			LayerContentChangeEvent changeEvent(this, layerItem);
			raiseEvent(&changeEvent);
		}
		else
			layerItem->setSelected(!layerItem->isSelected());
	}

	// Deselect all others if shift key isn't pressed.
	if (!(event->getKeyState() & KsShift))
	{
		RefArray< LayerItem > items;
		getItems(items, GfDefault | GfDescendants);

		for (RefArray< LayerItem >::iterator j = items.begin(); j != items.end(); ++j)
		{
			if (*j != layerItem)
				(*j)->setSelected(false);
		}
	}

	// Set focus and redraw.
	setFocus();
	update();

	SelectionChangeEvent selectionChangeEvent(this);
	raiseEvent(&selectionChangeEvent);

	event->consume();
}

void LayerControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	int scrollWidth = m_scrollBar->getPreferedSize().cx;
	int scrollOffset = m_scrollBar->getPosition();

	Rect rcInner = getInnerRect();
	rcInner.right -= scrollWidth;

	canvas.fillRect(rcInner);

	Rect rcItem(rcInner.left, rcInner.top - scrollOffset, rcInner.right, rcInner.top - scrollOffset + c_layerItemHeight);
	for (RefArray< LayerItem >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		paintItem(canvas, rcItem, *i, 0);

	event->consume();
}

		}
	}
}
