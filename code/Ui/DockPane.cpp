/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/MathUtils.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/DockPane.h"
#include "Ui/FloodLayout.h"
#include "Ui/StyleSheet.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"

namespace traktor::ui
{
	namespace
	{

const int c_splitterDim = 6;

int32_t calculateRealSplit(const Widget* widget, int32_t gripperDim, const Rect& rc, int32_t split, bool vertical)
{
	const int32_t minimumSplit = gripperDim + widget->pixel(64_ut);
	const int32_t dim = vertical ? rc.getHeight() : rc.getWidth();
	int32_t realSplit = split >= 0 ? split : dim + split;
	if (realSplit <= minimumSplit)
		realSplit = minimumSplit;
	if (realSplit >= dim - minimumSplit)
		realSplit = dim - minimumSplit;
	realSplit += vertical ? rc.top : rc.left;
	return realSplit;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.DockPane", DockPane, Object)

DockPane::DockPane(Widget* owner, DockPane* parent)
:	m_owner(owner)
,	m_parent(parent)
{
	m_bitmapClose = new ui::StyleBitmap(L"UI.DockClose");
	T_FATAL_ASSERT (m_bitmapClose);

	m_bitmapGripper = new ui::StyleBitmap(L"UI.DockGripper");
	T_FATAL_ASSERT (m_bitmapGripper);

	m_gripperDim = owner->getFont().getSize() + 9_ut;
}

void DockPane::split(bool vertical, Unit split, Ref< DockPane >& outLeftPane, Ref< DockPane >& outRightPane)
{
	T_FATAL_ASSERT_M(m_widgets.empty(), L"Cannot split pane containing widgets.");
	T_FATAL_ASSERT_M(m_child[0] == nullptr && m_child[1] == nullptr, L"Pane already split.");

	outLeftPane = new DockPane(m_owner, this);
	outRightPane = new DockPane(m_owner, this);

	m_split = split;
	m_vertical = vertical;
	m_child[0] = outLeftPane;
	m_child[1] = outRightPane;
}

void DockPane::dock(Widget* widget)
{
	T_FATAL_ASSERT_M(widget != nullptr, L"Cannot dock null widget.");
	T_FATAL_ASSERT_M(m_child[0] == nullptr && m_child[1] == nullptr, L"Pane already split.");
	T_FATAL_ASSERT_M(m_stackable || m_widgets.empty(), L"Pane not stackable; already contain a widget.");

	m_widgets.push_back({ widget, 0, 0, 0, true });
	m_child[0] =
	m_child[1] = nullptr;

	synchronizeVisibility();
}

void DockPane::dock(Widget* widget, Direction direction, Unit split)
{
	if (!m_widgets.empty())
	{
		AlignedVector< WidgetInfo > currentWidgets = m_widgets;
		bool currentDetachable = m_detachable;

		m_widgets.resize(0);
		m_child[0] = nullptr;
		m_child[1] = nullptr;
		m_detachable = false;

		Ref< DockPane > leftPane, rightPane;
		DockPane::split(
			direction == DrNorth || direction == DrSouth,
			(direction == DrNorth || direction == DrWest) ? split : -split,
			leftPane,
			rightPane
		);

		if (direction == DrNorth || direction == DrWest)
		{
			leftPane->dock(widget);
			
			for (const auto& w : currentWidgets)
				rightPane->dock(w.widget);

			rightPane->setDetachable(currentDetachable);
		}
		else
		{
			rightPane->dock(widget);
			
			for (const auto& w : currentWidgets)
				leftPane->dock(w.widget);

			leftPane->setDetachable(currentDetachable);
		}
	}
	else
	{
		if (m_child[0] && m_child[1])
		{
			Ref< DockPane > paneLeft = new DockPane(m_owner, this);
			Ref< DockPane > paneRight = new DockPane(m_owner, this);

			if (direction == DrNorth || direction == DrWest)
			{
				paneLeft->dock(widget);

				paneRight->m_widgets = m_widgets;
				paneRight->m_detachable = m_detachable;
				paneRight->m_child[0] = m_child[0];
				paneRight->m_child[1] = m_child[1];
				paneRight->m_vertical = m_vertical;
				paneRight->m_split = m_split;

				if (paneRight->m_child[0])
					paneRight->m_child[0]->m_parent = paneRight;
				if (paneRight->m_child[1])
					paneRight->m_child[1]->m_parent = paneRight;

				m_child[0] = paneLeft;
				m_child[1] = paneRight;
				m_vertical = direction == DrNorth;
				m_split = split;
			}
			else
			{
				paneLeft->m_widgets = m_widgets;
				paneLeft->m_detachable = m_detachable;
				paneLeft->m_child[0] = m_child[0];
				paneLeft->m_child[1] = m_child[1];
				paneLeft->m_vertical = m_vertical;
				paneLeft->m_split = m_split;

				if (paneLeft->m_child[0])
					paneLeft->m_child[0]->m_parent = paneLeft;
				if (paneLeft->m_child[1])
					paneLeft->m_child[1]->m_parent = paneLeft;

				paneRight->dock(widget);

				m_child[0] = paneLeft;
				m_child[1] = paneRight;
				m_vertical = direction == DrSouth;
				m_split = -split;
			}
		}
		else
		{
			T_ASSERT(!m_child[0] && !m_child[1]);
			dock(widget);

			if (m_parent)
			{
				if (m_parent->m_split < 0_ut)
					m_parent->m_split = Unit(-traktor::abs(split.get()));
				else
					m_parent->m_split = Unit(traktor::abs(split.get()));
			}
		}
	}
}

void DockPane::undock(Widget* widget)
{
	T_FATAL_ASSERT_M(widget != nullptr, L"Cannot un-dock null widget.");

	auto it = std::find_if(m_widgets.begin(), m_widgets.end(), [&](const WidgetInfo& w) { return w.widget == widget; });
	if (it != m_widgets.end())
	{
		m_widgets.erase(it);
		if (m_widgets.empty())
		{
			m_child[0] = nullptr;
			m_child[1] = nullptr;
		}
	}
	else if (m_child[0])
	{
		T_FATAL_ASSERT(m_child[1]);

		m_child[0]->undock(widget);
		m_child[1]->undock(widget);

		if (
			m_child[0]->m_widgets.empty() &&
			m_child[1]->m_widgets.empty() &&
			m_child[0]->m_child[0] == nullptr &&
			m_child[1]->m_child[0] == nullptr
		)
		{
			T_ASSERT(m_child[0]->m_child[1] == nullptr);
			T_ASSERT(m_child[1]->m_child[1] == nullptr);

			m_child[0] = nullptr;
			m_child[1] = nullptr;
		}
		else if (
			!m_child[1]->m_widgets.empty() &&
			m_child[0]->m_widgets.empty() &&
			m_child[0]->m_child[0] == nullptr
		)
		{
			T_ASSERT(m_child[0]->m_child[1] == nullptr);

			m_widgets = m_child[1]->m_widgets;
			m_detachable = m_child[1]->m_detachable;
			m_stackable = m_child[1]->m_stackable;

			m_child[0] = nullptr;
			m_child[1] = nullptr;
		}
		else if (
			!m_child[0]->m_widgets.empty() &&
			m_child[1]->m_widgets.empty() &&
			m_child[1]->m_child[0] == nullptr
		)
		{
			T_ASSERT(m_child[1]->m_child[1] == nullptr);

			m_widgets = m_child[0]->m_widgets;
			m_detachable = m_child[0]->m_detachable;
			m_stackable = m_child[0]->m_stackable;

			m_child[0] = nullptr;
			m_child[1] = nullptr;
		}
	}

	synchronizeVisibility();
}

void DockPane::detach()
{
	T_ASSERT(m_detachable);

	if (m_parent)
	{
		Ref< DockPane > childPane;

		if (m_parent->m_child[0] == this)
			childPane = m_parent->m_child[1];
		else if (m_parent->m_child[1] == this)
			childPane = m_parent->m_child[0];

		T_ASSERT(childPane);

		m_parent->m_widgets = childPane->m_widgets;
		m_parent->m_child[0] = childPane->m_child[0];
		m_parent->m_child[1] = childPane->m_child[1];
		m_parent->m_split = childPane->m_split;
		m_parent->m_detachable = childPane->m_detachable;
		m_parent->m_stackable = childPane->m_stackable;
		m_parent->m_vertical = childPane->m_vertical;

		if (m_parent->m_child[0])
			m_parent->m_child[0]->m_parent = m_parent;
		if (m_parent->m_child[1])
			m_parent->m_child[1]->m_parent = m_parent;

		m_parent->synchronizeVisibility();
	}

	m_parent = nullptr;
	m_widgets.resize(0);
	m_child[0] = nullptr;
	m_child[1] = nullptr;
}

void DockPane::showTab(int32_t tab)
{
	T_FATAL_ASSERT_M(m_stackable, L"Cannot show tab on non-stackable pane.");

	for (int32_t i = 0; i < (int32_t)m_widgets.size(); ++i)
		m_widgets[i].tabDepth++;

	for (int32_t i = 0; i < (int32_t)m_widgets.size(); ++i)
	{
		if (!m_widgets[i].visible)
			continue;
		if (tab-- == 0)
			m_widgets[i].tabDepth = 0;
	}

	synchronizeVisibility();
}

void DockPane::showWidget(Widget* widget)
{
	const auto it = std::find_if(m_widgets.begin(), m_widgets.end(), [&](const WidgetInfo& w) { return w.widget == widget; });
	T_FATAL_ASSERT_M(it != m_widgets.end(), L"Widget not docked in this pane.");
	it->visible = true;
	synchronizeVisibility();
}

void DockPane::hideWidget(Widget* widget)
{
	const auto it = std::find_if(m_widgets.begin(), m_widgets.end(), [&](const WidgetInfo& w) { return w.widget == widget; });
	T_FATAL_ASSERT_M(it != m_widgets.end(), L"Widget not docked in this pane.");
	it->visible = false;
	synchronizeVisibility();
}

bool DockPane::isWidgetVisible(const Widget* widget) const
{
	const auto it = std::find_if(m_widgets.begin(), m_widgets.end(), [&](const WidgetInfo& w) { return w.widget == widget; });
	T_FATAL_ASSERT_M(it != m_widgets.end(), L"Widget not docked in this pane.");
	return it->visible;
}

DockPane* DockPane::findWidgetPane(const Widget* widget)
{
	if (std::find_if(m_widgets.begin(), m_widgets.end(), [&](const WidgetInfo& w) { return w.widget == widget; }) != m_widgets.end())
		return this;

	for (uint32_t i = 0; i < 2; ++i)
	{
		if (!m_child[i])
			continue;

		DockPane* pane = m_child[i]->findWidgetPane(widget);
		if (pane)
			return pane;
	}

	return nullptr;
}

DockPane* DockPane::getPaneFromPosition(const Point& position)
{
	if (!m_rect.inside(position))
		return nullptr;

	for (int i = 0; i < 2; ++i)
	{
		if (m_child[i])
		{
			DockPane* childPane = m_child[i]->getPaneFromPosition(position);
			if (childPane)
				return childPane;
		}
	}

	//if (m_widget && m_widget->isVisible(false))
	if (!m_widgets.empty())
		return this;
	else
		return nullptr;
}

DockPane* DockPane::getSplitterFromPosition(const Point& position)
{
	if (!m_rect.inside(position))
		return nullptr;

	for (int i = 0; i < 2; ++i)
	{
		if (m_child[i])
		{
			DockPane* childPane = m_child[i]->getSplitterFromPosition(position);
			if (childPane)
				return childPane;
		}
	}

	//if (m_widget && m_widget->isVisible(false))
	if (!m_widgets.empty())
		return nullptr;
	else
		return this;
}

bool DockPane::hitGripper(const Point& position) const
{
	T_ASSERT(m_rect.inside(position));

	if (isSplitter()) // || (m_widget && !m_widget->isVisible(false)))
		return false;

	return position.y >= m_rect.top && position.y <= m_rect.top + m_owner->pixel(m_gripperDim);
}

bool DockPane::hitGripperClose(const Point& position) const
{
	T_ASSERT(m_rect.inside(position));

	if (m_stackable || isSplitter() || !hitGripper(position))
		return false;

	const int32_t closeWidth = m_bitmapClose->getSize(m_owner).cx;
	return position.x >= m_rect.right - closeWidth - m_owner->pixel(4_ut);
}

bool DockPane::hitSplitter(const Point& position) const
{
	T_ASSERT(m_rect.inside(position));

	if (!isSplitter())
		return false;

	int32_t pos, split;
	if (m_vertical)
	{
		pos = position.y;
		split = calculateRealSplit(m_owner, m_owner->pixel(m_gripperDim), m_rect, m_owner->pixel(m_split), true);
	}
	else
	{
		pos = position.x;
		split = calculateRealSplit(m_owner, m_owner->pixel(m_gripperDim), m_rect, m_owner->pixel(m_split), false);
	}

	return pos >= split - c_splitterDim / 2 && pos <= split + c_splitterDim / 2;
}

int32_t DockPane::hitTab(const Point& position) const
{
	T_ASSERT(m_rect.inside(position));

	if (m_widgets.size() <= 1)
		return -1;

	if (isSplitter() || !hitGripper(position))
		return false;

	for (int32_t i = 0; i < (int32_t)m_widgets.size(); ++i)
	{
		const auto& w = m_widgets[i];
		if (position.x >= w.tabMin && position.x < w.tabMax)
			return i;
	}

	return -1;
}

void DockPane::setSplitterPosition(const Point& position)
{
	int32_t pos = m_vertical ? (position.y - m_rect.top) : (position.x - m_rect.left);
	int32_t extent = m_vertical ? m_rect.getHeight() : m_rect.getWidth();

	pos = std::max(0, pos);
	pos = std::min(extent, pos);

	if (m_split < 0_ut)
		m_split = m_owner->unit(-(extent - pos));
	else
		m_split = m_owner->unit(pos);
}

void DockPane::setDetachable(bool detachable)
{
	m_detachable = detachable;
}

bool DockPane::isDetachable() const
{
	return m_detachable;
}

void DockPane::setStackable(bool stackable)
{
	m_stackable = stackable;
}

bool DockPane::isStackable() const
{
	return m_stackable;
}

bool DockPane::isVisible() const
{
	if (isSplitter())
		return m_child[0]->isVisible() || m_child[1]->isVisible();

	for (const auto& w : m_widgets)
	{
		if (w.visible)
			return true;
	}

	return false;
}

void DockPane::synchronizeVisibility()
{
	if (m_widgets.empty())
		return;

	if (m_stackable)
	{
		// Show single widget with lowest tab-depth.
		int32_t minTabDepth = std::numeric_limits< int32_t >::max();
		int32_t minTabIndex = 0;

		for (int32_t i = 0; i < (int32_t)m_widgets.size(); ++i)
		{
			const WidgetInfo& w = m_widgets[i];
			if (!w.visible)
				continue;

			if (w.tabDepth < minTabDepth)
			{
				minTabIndex = i;
				minTabDepth = w.tabDepth;
			}
		}

		for (int32_t i = 0; i < (int32_t)m_widgets.size(); ++i)
		{
			const WidgetInfo& w = m_widgets[i];
			w.widget->setVisible(i == minTabIndex);
		}
	}
	else
	{
		// Only a single widget is allowed.
		WidgetInfo& w = m_widgets.front();
		w.widget->setVisible(w.visible);
	}
}

void DockPane::update(const Rect& rect, AlignedVector< WidgetRect >& outWidgetRects)
{
	m_rect = rect;
	if (!m_widgets.empty())
	{
		Rect widgetRect = rect;
		if (m_detachable)
			widgetRect.top += m_owner->pixel(m_gripperDim);
		for (const auto& w : m_widgets)
			outWidgetRects.push_back(WidgetRect(w.widget, widgetRect));
	}
	else
	{
		const bool childVisible1 = m_child[0] ? m_child[0]->isVisible() : false;
		const bool childVisible2 = m_child[1] ? m_child[1]->isVisible() : false;

		if (childVisible1 && childVisible2)
		{
			Rect childRects[2] = { rect, rect };
			if (m_vertical)
			{
				const int32_t split = calculateRealSplit(m_owner, m_owner->pixel(m_gripperDim), rect, m_owner->pixel(m_split), true);
				childRects[0].bottom = split - c_splitterDim / 2;
				childRects[1].top = split + c_splitterDim / 2;
			}
			else
			{
				const int32_t split = calculateRealSplit(m_owner, m_owner->pixel(m_gripperDim), rect, m_owner->pixel(m_split), false);
				childRects[0].right = split - c_splitterDim / 2;
				childRects[1].left = split + c_splitterDim / 2;
			}
			if (m_child[0])
				m_child[0]->update(childRects[0], outWidgetRects);
			if (m_child[1])
				m_child[1]->update(childRects[1], outWidgetRects);
		}
		else
		{
			if (m_child[0] && childVisible1)
				m_child[0]->update(rect, outWidgetRects);
			if (m_child[1] && childVisible2)
				m_child[1]->update(rect, outWidgetRects);
		}
	}
}

void DockPane::draw(Canvas& canvas)
{
	if (!isVisible())
		return;

	const StyleSheet* ss = m_owner->getStyleSheet();

	// Draw splitter.
	if (isSplitter() && m_child[0]->isVisible() && m_child[1]->isVisible())
	{
		Rect splitterRect = m_rect;
		if (m_vertical)
		{
			const int32_t split = calculateRealSplit(m_owner, m_owner->pixel(m_gripperDim), m_rect, m_owner->pixel(m_split), true);
			splitterRect.left += 2;
			splitterRect.right -= 2;
			splitterRect.top = split - c_splitterDim / 2;
			splitterRect.bottom = split + c_splitterDim / 2;
			canvas.setBackground(ss->getColor(this, L"splitter-color"));
			canvas.fillRect(splitterRect);
		}
		else
		{
			const int32_t split = calculateRealSplit(m_owner, m_owner->pixel(m_gripperDim), m_rect, m_owner->pixel(m_split), false);
			splitterRect.left = split - 1;
			splitterRect.right = split + 1;
			splitterRect.top += 2;
			splitterRect.bottom -= 2;
			canvas.setBackground(ss->getColor(this, L"splitter-color"));
			canvas.fillRect(splitterRect);
		}
	}

	if (m_detachable && !m_widgets.empty())
	{
		const FontMetric fm = m_owner->getFontMetric();

		Rect captionRect = m_rect;
		captionRect.bottom = captionRect.top + m_owner->pixel(m_gripperDim);

		const int32_t closeWidth = m_bitmapClose->getSize(m_owner).cx;

		Rect titleRect = captionRect; // .offset(0, -1);
		titleRect.right -= closeWidth + m_owner->pixel(4_ut);

		if (m_stackable)
		{
			// Multiple widgets stacked; draw tabs.
			int32_t left = titleRect.left;
			for (size_t i = 0; i < m_widgets.size(); ++i)
			{
				WidgetInfo& w = m_widgets[i];
				if (!w.visible)
					continue;

				const std::wstring title = w.widget->getText();
				const Size titleExtent = fm.getExtent(title);

				const Rect rcTab(
					left, titleRect.top,
					left + titleExtent.cx + m_owner->pixel(12_ut), titleRect.bottom
				);

				const bool visible = w.widget->isVisible(false);
				if (visible)
				{
					canvas.setForeground(ss->getColor(this, L"tab-color-active"));
					canvas.setBackground(ss->getColor(this, L"tab-selected-background-color"));
				}
				else
				{
					canvas.setForeground(ss->getColor(this, L"tab-color-inactive"));
					canvas.setBackground(ss->getColor(this, L"tab-background-color"));
				}
				canvas.fillRect(rcTab);

				Rect rcTabTitle = rcTab;
				rcTabTitle.left += m_owner->pixel(4_ut);
				canvas.drawText(rcTabTitle, title, AnLeft, AnCenter);

				w.tabMin = left;
				w.tabMax = left + rcTab.getWidth();

				left += rcTab.getWidth();
			}

			titleRect.left = left;
		}
		else
		{
			// Only a single widget docked.
			Widget* widget = m_widgets.front().widget;

			const std::wstring title = widget->getText();
			const Size titleExtent = fm.getExtent(title);

			titleRect.left += m_owner->pixel(4_ut);
			canvas.setForeground(ss->getColor(this, L"color"));
			canvas.drawText(titleRect, title, AnLeft, AnCenter);
			titleRect.left += titleExtent.cx;
		}

		// Draw gripper.
		int32_t gx = titleRect.left + m_owner->pixel(4_ut);
		int32_t gx1 = captionRect.right - (m_stackable ? 0 : closeWidth) - m_owner->pixel(4_ut);
		const int32_t gw = m_bitmapGripper->getSize(m_owner).cx;
		const int32_t gh = m_bitmapGripper->getSize(m_owner).cy;

		canvas.setBackground(ss->getColor(this, L"gripper-color"));
		while (gx < gx1)
		{
			const int32_t w = min(gw, gx1 - gx);
			canvas.drawBitmap(
				Point(gx, captionRect.getCenter().y - gh / 2),
				Point(0, 0),
				Size(w, gh),
				m_bitmapGripper,
				BlendMode::Modulate
			);
			gx += gw;
		}

		// Draw close button.
		if (!m_stackable)
		{
			canvas.drawBitmap(
				Point(captionRect.right - closeWidth - m_owner->pixel(4_ut), captionRect.getCenter().y - m_bitmapClose->getSize(m_owner).cy / 2),
				Point(0, 0),
				m_bitmapClose->getSize(m_owner),
				m_bitmapClose,
				BlendMode::Alpha
			);
		}
	}

	if (m_child[0])
		m_child[0]->draw(canvas);
	if (m_child[1])
		m_child[1]->draw(canvas);
}

}
