#include "Core/Math/Color4ub.h"
#include "Ui/Application.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Graph/GraphCanvas.h"
#include "Ui/Graph/Group.h"
#include "Ui/Graph/PaintSettings.h"

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
	const Rect rcTitle = rc.inflate(-dpi96(16), -dpi96(4));

	// Draw group shape.
	{
		const int32_t imageIndex = isSelected() ? 1 : 0;
		const Size sz = m_image[imageIndex]->getSize();

		const int32_t tw = sz.cx / 3;
		const int32_t th = sz.cy / 3;

		const int32_t sx[] = { 0, tw, sz.cx - tw, sz.cx };
		const int32_t sy[] = { 0, th, sz.cy - th, sz.cy };

		const int32_t dw = rc.getWidth(); //  -dpi96(c_marginWidth) * 2;
		const int32_t dh = rc.getHeight();

		const int32_t dx[] = { 0, tw, dw - tw, dw };
		const int32_t dy[] = { 0, th, dh - th, dh };

		for (int32_t iy = 0; iy < 3; ++iy)
		{
			for (int32_t ix = 0; ix < 3; ++ix)
			{
				canvas->drawBitmap(
					rc.getTopLeft() + Size(/*dpi96(c_marginWidth)*/0, 0) + Size(dx[ix], dy[iy]),
					Size(dx[ix + 1] - dx[ix], dy[iy + 1] - dy[iy]),
					Point(sx[ix], sy[iy]),
					Size(sx[ix + 1] - sx[ix], sy[iy + 1] - sy[iy]),
					m_image[imageIndex],
					BlendMode::Alpha
				);
			}
		}
	}

	// Draw title.
	canvas->setFont(canvas->getPaintSettings().getFontGroup());
	canvas->setForeground(Color4ub(255, 255, 255, 180));
	canvas->drawText(rcTitle, m_title, AnLeft, AnTop);
	canvas->setFont(canvas->getPaintSettings().getFont());
}

Group::Group(GraphControl* owner, const std::wstring& title, const Point& position, const Size& size)
: m_owner(owner)
, m_title(title)
, m_position(position)
, m_size(size)
{
	m_image[0] = new StyleBitmap(L"UI.Graph.Group");
	m_image[1] = new StyleBitmap(L"UI.Graph.GroupSelected");
}

}
