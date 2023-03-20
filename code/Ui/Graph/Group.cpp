#include "Core/Math/Color4ub.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/Group.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Group", Group, Object)

void Group::setTitle(const std::wstring& title)
{
	m_title = title;
}

const std::wstring& Group::getTitle() const
{
	return m_title;
}

void Group::setPosition(const Point& position)
{
	m_position = position;
}

const Point& Group::getPosition() const
{
	return m_position;
}

void Group::setSize(const Size& size)
{
	m_size = size;
}

const Size& Group::getSize() const
{
	return m_size;
}

bool Group::hit(const Point& p) const
{
	return calculateRect().inside(p);
}

Rect Group::calculateRect() const
{
	return Rect(m_position, m_size);
}

void Group::setSelected(bool selected)
{
	m_selected = selected;
}

bool Group::isSelected() const
{
	return m_selected;
}

void Group::paint(GraphCanvas* canvas, const Size& offset) const
{
	const Rect rc = calculateRect().offset(offset);

	canvas->setForeground(Color4ub(255, 255, 255, 255));
	canvas->setBackground(Color4ub(255, 255, 255, 32));
	canvas->fillRect(rc);
	canvas->drawRect(rc);

	canvas->setBackground(Color4ub(255, 255, 255, 255));
	canvas->drawText(rc, m_title, AnLeft, AnTop);
}

}
