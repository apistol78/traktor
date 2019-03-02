#include <sstream>
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/PropertyList/ColorPropertyItem.h"
#include "Ui/PropertyList/PropertyList.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ColorPropertyItem", ColorPropertyItem, PropertyItem)

ColorPropertyItem::ColorPropertyItem(const std::wstring& text, const Color4f& value, bool hdr)
:	PropertyItem(text)
,	m_value(value)
,	m_hdr(hdr)
,	m_rcColor(0, 0, 0, 0)
{
}

void ColorPropertyItem::setValue(const Color4f& value)
{
	m_value = value;
}

const Color4f& ColorPropertyItem::getValue() const
{
	return m_value;
}

bool ColorPropertyItem::getHighDynamicRange() const
{
	return m_hdr;
}

void ColorPropertyItem::mouseButtonUp(MouseButtonUpEvent* event)
{
	if (m_rcColor.inside(event->getPosition()))
		notifyCommand(Command(L"Property.Edit"));
}

void ColorPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	m_rcColor = Rect(rc.left + 2, rc.top + 2, rc.left + dpi96(22), rc.bottom - 2);

	if (m_hdr)
	{
		float ev = m_value.getEV();

		Color4f c0 = m_value;
		c0.setEV(Scalar(0.0f));

		float r = c0.getRed();
		float g = c0.getGreen();
		float b = c0.getBlue();
		float a = c0.getAlpha();

		int32_t ir = int32_t(r * 255.0f);
		int32_t ig = int32_t(g * 255.0f);
		int32_t ib = int32_t(b * 255.0f);
		int32_t ia = int32_t(a * 255.0f);

		std::wstringstream ss;
		ss << ir << L", " << ig << L", " << ib << L", " << ia << L" (EV " << (ev > 0.0f ? L"+" : L"") << ev << L")";
		canvas.drawText(rc.inflate(-2, 0).offset(dpi96(22), 0), ss.str(), AnLeft, AnCenter);

		// Ignore alpha when drawing color preview.
		canvas.setBackground(Color4ub(ir, ig, ib, 255));
		canvas.setForeground(Color4ub(0, 0, 0));

		// Draw color preview with a black border.
		canvas.fillRect(m_rcColor);
		canvas.drawRect(m_rcColor);
	}
	else
	{
		float r = m_value.getRed();
		float g = m_value.getGreen();
		float b = m_value.getBlue();
		float a = m_value.getAlpha();

		int32_t ir = int32_t(r * 255.0f);
		int32_t ig = int32_t(g * 255.0f);
		int32_t ib = int32_t(b * 255.0f);
		int32_t ia = int32_t(a * 255.0f);

		std::wstringstream ss;
		ss << ir << L", " << ig << L", " << ib << L", " << ia;
		canvas.drawText(rc.inflate(-2, 0).offset(dpi96(22), 0), ss.str(), AnLeft, AnCenter);

		// Ignore alpha when drawing color preview.
		canvas.setBackground(Color4ub(ir, ig, ib, 255));
		canvas.setForeground(Color4ub(0, 0, 0));

		// Draw color preview with a black border.
		canvas.fillRect(m_rcColor);
		canvas.drawRect(m_rcColor);
	}
}

	}
}
