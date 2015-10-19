#include "Core/Math/Bezier2nd.h"
#include "Flash/Editor/FlashPathControl.h"
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashPathControl", FlashPathControl, ui::Widget)

bool FlashPathControl::create(ui::Widget* parent, int style)
{
	if (!ui::Widget::create(parent, style))
		return false;

	addEventHandler< ui::PaintEvent >(this, &FlashPathControl::eventPaint);
	return true;
}

void FlashPathControl::setPath(const Path& path)
{
	m_path = path;
	update();
}

void FlashPathControl::eventPaint(ui::PaintEvent* event)
{
	ui::Canvas& canvas = event->getCanvas();
	ui::Rect rc = getInnerRect();

	const ui::StyleSheet* ss = ui::Application::getInstance()->getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rc);

	rc = rc.inflate(-8, -8);

	Aabb2 bounds = m_path.getBounds();

	canvas.setBackground(Color4ub(255, 0, 0, 255));
	canvas.setForeground(Color4ub(0, 0, 0, 255));

	const AlignedVector< Vector2 >& points = m_path.getPoints();
	const AlignedVector< SubPath >& subPaths = m_path.getSubPaths();

	for (uint32_t i = 0; i < uint32_t(points.size()); ++i)
	{
		Vector2 pt = points[i];

		pt = (pt - bounds.mn) / (bounds.mx - bounds.mn);
		pt = pt * Vector2(rc.getWidth(), rc.getHeight());

		canvas.fillCircle(ui::Point(pt.x, pt.y), 4);
	}

	for (uint32_t i = 0; i < uint32_t(subPaths.size()); ++i)
	{
		const SubPath& sp = subPaths[i];

		//Vector2 pt = points[sp.segments[0].pointsOffset];
		//pt = (pt - bounds.mn) / (bounds.mx - bounds.mn);
		//pt = pt * Vector2(rc.getWidth(), rc.getHeight());
		//canvas.drawText(ui::Point(pt.x, pt.y), 

		for (uint32_t j = 0; j < uint32_t(sp.segments.size()); ++j)
		{
			const SubPathSegment& sps = sp.segments[j];
			if (sps.type == SpgtLinear)
			{
				Vector2 pt0 = points[sps.pointsOffset + 0];
				Vector2 pt1 = points[sps.pointsOffset + 1];

				pt0 = (pt0 - bounds.mn) / (bounds.mx - bounds.mn);
				pt0 = pt0 * Vector2(rc.getWidth(), rc.getHeight());
				pt1 = (pt1 - bounds.mn) / (bounds.mx - bounds.mn);
				pt1 = pt1 * Vector2(rc.getWidth(), rc.getHeight());

				canvas.drawLine(
					ui::Point(pt0.x, pt0.y),
					ui::Point(pt1.x, pt1.y)
				);
			}
			else if (sps.type == SpgtQuadratic)
			{
				Vector2 pt0 = points[sps.pointsOffset + 0];
				Vector2 ptc = points[sps.pointsOffset + 1];
				Vector2 pt1 = points[sps.pointsOffset + 2];

				pt0 = (pt0 - bounds.mn) / (bounds.mx - bounds.mn);
				pt0 = pt0 * Vector2(rc.getWidth(), rc.getHeight());
				ptc = (ptc - bounds.mn) / (bounds.mx - bounds.mn);
				ptc = ptc * Vector2(rc.getWidth(), rc.getHeight());
				pt1 = (pt1 - bounds.mn) / (bounds.mx - bounds.mn);
				pt1 = pt1 * Vector2(rc.getWidth(), rc.getHeight());

				Bezier2nd b(pt0, ptc, pt1);
				for (int32_t t = 0; t < 10; ++t)
				{
					float f0 = float(t / 10.0f);
					float f1 = float((t + 1) / 10.0f);

					Vector2 p0 = b.evaluate(f0);
					Vector2 p1 = b.evaluate(f1);

					canvas.drawLine(
						ui::Point(p0.x, p0.y),
						ui::Point(p1.x, p1.y)
					);
				}
			}
		}
	}

	event->consume();
}

	}
}
