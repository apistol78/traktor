#include <algorithm>
#include "Core/Math/MathUtils.h"
#include "Ui/Custom/QuadSplitter.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.QuadSplitter", QuadSplitter, Widget)

const int c_splitterSize = 2;
const int c_relativeScale = 100000;

QuadSplitter::QuadSplitter()
:	m_vertical(false)
,	m_position(0, 0)
,	m_negativeX(false)
,	m_negativeY(false)
,	m_relative(false)
,	m_border(0)
,	m_drag(0)
{
}

bool QuadSplitter::create(Widget* parent, const Point& position, bool relative, int border)
{
	if (!Widget::create(parent))
		return false;

	m_position.x = (position.x < 0) ? -position.x : position.x;
	m_position.y = (position.y < 0) ? -position.y : position.y;
	m_negativeX = (position.x < 0) ? true : false;
	m_negativeY = (position.y < 0) ? true : false;
	m_relative = relative;
	m_border = border;
	m_drag = 0;

	if (relative)
	{
		m_position.x *= c_relativeScale / 100;
		m_position.y *= c_relativeScale / 100;
	}

	addEventHandler< MouseMoveEvent >(this, &QuadSplitter::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &QuadSplitter::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &QuadSplitter::eventButtonUp);
	addEventHandler< SizeEvent >(this, &QuadSplitter::eventSize);

	return true;
}

Size QuadSplitter::getMinimumSize() const
{
	Ref< Widget > widgets[4];
	getWidgets(widgets);

	Size size(c_splitterSize, c_splitterSize);
	size.cx += std::max(
		widgets[0] ? widgets[0]->getMinimumSize().cx : 0,
		widgets[3] ? widgets[3]->getMinimumSize().cx : 0
	);
	size.cx += std::max(
		widgets[1] ? widgets[1]->getMinimumSize().cx : 0,
		widgets[2] ? widgets[2]->getMinimumSize().cx : 0
	);
	size.cy += std::max(
		widgets[0] ? widgets[0]->getMinimumSize().cy : 0,
		widgets[1] ? widgets[1]->getMinimumSize().cy : 0
	);
	size.cy += std::max(
		widgets[2] ? widgets[2]->getMinimumSize().cy : 0,
		widgets[3] ? widgets[3]->getMinimumSize().cy : 0
	);

	return size;
}

Size QuadSplitter::getPreferedSize() const
{
	Ref< Widget > widgets[4];
	getWidgets(widgets);

	Size size(c_splitterSize, c_splitterSize);
	size.cx += std::max(
		widgets[0] ? widgets[0]->getPreferedSize().cx : 0,
		widgets[3] ? widgets[3]->getPreferedSize().cx : 0
	);
	size.cx += std::max(
		widgets[1] ? widgets[1]->getPreferedSize().cx : 0,
		widgets[2] ? widgets[2]->getPreferedSize().cx : 0
	);
	size.cy += std::max(
		widgets[0] ? widgets[0]->getPreferedSize().cy : 0,
		widgets[1] ? widgets[1]->getPreferedSize().cy : 0
	);
	size.cy += std::max(
		widgets[2] ? widgets[2]->getPreferedSize().cy : 0,
		widgets[3] ? widgets[3]->getPreferedSize().cy : 0
	);

	return size;
}

Size QuadSplitter::getMaximumSize() const
{
	Ref< Widget > widgets[4];
	getWidgets(widgets);

	Size size(c_splitterSize, c_splitterSize);
	size.cx += std::max(
		widgets[0] ? widgets[0]->getMaximumSize().cx : 0,
		widgets[3] ? widgets[3]->getMaximumSize().cx : 0
	);
	size.cx += std::max(
		widgets[1] ? widgets[1]->getMaximumSize().cx : 0,
		widgets[2] ? widgets[2]->getMaximumSize().cx : 0
	);
	size.cy += std::max(
		widgets[0] ? widgets[0]->getMaximumSize().cy : 0,
		widgets[1] ? widgets[1]->getMaximumSize().cy : 0
	);
	size.cy += std::max(
		widgets[2] ? widgets[2]->getMaximumSize().cy : 0,
		widgets[3] ? widgets[3]->getMaximumSize().cy : 0
	);

	return size;
}

void QuadSplitter::update(const Rect* rc, bool immediate)
{
	Rect innerRect = getInnerRect();
	Point splitterPosition = getAbsolutePosition();

	Ref< Widget > widgets[4];
	getWidgets(widgets);

	Rect widgetRects[] =
	{
		Rect(
			innerRect.left + splitterPosition.x + c_splitterSize / 2,
			innerRect.top,
			innerRect.right,
			innerRect.top + splitterPosition.y - c_splitterSize / 2
		),
		Rect(
			innerRect.left,
			innerRect.top,
			innerRect.left + splitterPosition.x - c_splitterSize / 2,
			innerRect.top + splitterPosition.y - c_splitterSize / 2
		),
		Rect(
			innerRect.left,
			innerRect.top + splitterPosition.y + c_splitterSize / 2,
			innerRect.left + splitterPosition.x - c_splitterSize / 2,
			innerRect.bottom
		),
		Rect(
			innerRect.left + splitterPosition.x + c_splitterSize / 2,
			innerRect.top + splitterPosition.y + c_splitterSize / 2,
			innerRect.right,
			innerRect.bottom
		)
	};

	for (int i = 0; i < 4; ++i)
	{
		if (!widgets[i])
			continue;

		widgets[i]->setRect(widgetRects[i]);
		widgets[i]->update();
	}

	Widget::update(rc, immediate);
}

void QuadSplitter::setPosition(const Point& position)
{
	m_position = position;
	update();
}

Point QuadSplitter::getPosition() const
{
	if (m_relative)
		return Point((m_position.x * 100) / c_relativeScale, (m_position.y * 100) / c_relativeScale);
	else
		return m_position;
}

void QuadSplitter::getWidgets(Ref< Widget > outWidgets[4]) const
{
	Ref< Widget > child = getFirstChild();
	for (int i = 0; i < 4; ++i)
	{
		outWidgets[i] = child;
		if (child)
			child = child->getNextSibling();
	}
}

void QuadSplitter::setAbsolutePosition(const Point& position)
{
	Size innerSize = getInnerRect().getSize();

	m_position = position;

	m_position.x = std::max(m_position.x, m_border);
	m_position.x = std::min(m_position.x, innerSize.cx - m_border);

	m_position.y = std::max(m_position.y, m_border);
	m_position.y = std::min(m_position.y, innerSize.cy - m_border);

	if (m_negativeX)
		m_position.x = innerSize.cx - m_position.x;
	if (m_negativeY)
		m_position.y = innerSize.cy - m_position.y;

	if (m_relative)
	{
		m_position.x = (m_position.x * c_relativeScale) / innerSize.cx;
		m_position.y = (m_position.y * c_relativeScale) / innerSize.cy;
	}
}

Point QuadSplitter::getAbsolutePosition() const
{
	Size innerSize = getInnerRect().getSize();
	Point position = m_position;

	if (m_relative)
	{
		position.x = (position.x * innerSize.cx) / c_relativeScale;
		position.y = (position.y * innerSize.cy) / c_relativeScale;
	}

	if (m_negativeX)
		position.x = innerSize.cx - position.x;
	if (m_negativeY)
		position.y = innerSize.cy - position.y;

	return position;
}

void QuadSplitter::eventMouseMove(MouseMoveEvent* event)
{
	Point mousePosition = event->getPosition();
	Point splitterPosition = getAbsolutePosition();

	if (m_drag)
	{
		Size innerSize = getInnerRect().getSize();
		Point position = mousePosition;

		position.x = std::max(position.x, m_border);
		position.x = std::min(position.x, innerSize.cx - m_border);

		position.y = std::max(position.y, m_border);
		position.y = std::min(position.y, innerSize.cy - m_border);

		if (m_negativeX)
			position.x = innerSize.cx - m_position.x;
		if (m_negativeY)
			position.y = innerSize.cy - m_position.y;

		if (m_relative)
		{
			position.x = (position.x * c_relativeScale) / innerSize.cx;
			position.y = (position.y * c_relativeScale) / innerSize.cy;
		}

		if (m_drag & 1)
			m_position.x = position.x;
		if (m_drag & 2)
			m_position.y = position.y;

		update();
	}
	else
	{
		bool rangeX = traktor::abs(mousePosition.x - splitterPosition.x) <= c_splitterSize / 2;
		bool rangeY = traktor::abs(mousePosition.y - splitterPosition.y) <= c_splitterSize / 2;

		if (rangeX || rangeY)
		{
			if (rangeX && !rangeY)
				setCursor(CrSizeWE);
			else if (!rangeX && rangeY)
				setCursor(CrSizeNS);
			else
				setCursor(CrSizing);

			setCapture();
		}
		else
		{
			resetCursor();
			releaseCapture();
		}
	}
}

void QuadSplitter::eventButtonDown(MouseButtonDownEvent* event)
{
	Point mousePosition = event->getPosition();
	Point splitterPosition = getAbsolutePosition();

	bool rangeX = traktor::abs(mousePosition.x - splitterPosition.x) <= c_splitterSize / 2;
	bool rangeY = traktor::abs(mousePosition.y - splitterPosition.y) <= c_splitterSize / 2;

	if (rangeX || rangeY)
	{
		if (rangeX && !rangeY)
			setCursor(CrSizeWE);
		else if (!rangeX && rangeY)
			setCursor(CrSizeNS);
		else
			setCursor(CrSizing);

		setCapture();
		m_drag = (rangeX ? 1 : 0) | (rangeY ? 2 : 0);
	}
}

void QuadSplitter::eventButtonUp(MouseButtonUpEvent* event)
{
	if (m_drag)
	{
		releaseCapture();
		resetCursor();
		m_drag = 0;
	}
}

void QuadSplitter::eventSize(SizeEvent* event)
{
	update();
}

		}
	}
}
