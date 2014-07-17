#include <sstream>
#include "Core/Math/MathUtils.h"
#include "Ui/Custom/Envelope/EnvelopeContentChangeEvent.h"
#include "Ui/Custom/Envelope/EnvelopeControl.h"
#include "Ui/Custom/Envelope/EnvelopeEvaluator.h"
#include "Ui/Custom/Envelope/EnvelopeKey.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

float scaleValue(float value, float minValue, float maxValue)
{
	return clamp((value - minValue) / (maxValue - minValue), minValue, maxValue);
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.EnvelopeControl", EnvelopeControl, Widget)

bool EnvelopeControl::create(Widget* parent, EnvelopeEvaluator* evaluator, float minValue, float maxValue, int style)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &EnvelopeControl::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &EnvelopeControl::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &EnvelopeControl::eventMouseMove);
	addEventHandler< PaintEvent >(this, &EnvelopeControl::eventPaint);

	m_evaluator = evaluator;
	m_minValue = minValue;
	m_maxValue = maxValue;

	return true;
}

void EnvelopeControl::insertKey(EnvelopeKey* key)
{
	for (RefArray< EnvelopeKey >::iterator i = m_keys.begin(); i != m_keys.end(); ++i)
	{
		if (key->getT() < (*i)->getT())
		{
			m_keys.insert(i, key);
			return;
		}
	}
	m_keys.push_back(key);
}

const RefArray< EnvelopeKey >& EnvelopeControl::getKeys() const
{
	return m_keys;
}

void EnvelopeControl::addRange(const Color4ub& color, float limit0, float limit1, float limit2, float limit3)
{
	Range r = { color, { limit0, limit1, limit2, limit3 } };
	m_ranges.push_back(r);
}

void EnvelopeControl::eventButtonDown(MouseButtonDownEvent* event)
{
	Point pt = event->getPosition();

	m_selectedKey = 0;

	if (event->getButton() == MbtLeft)
	{
		for (RefArray< EnvelopeKey >::iterator i = m_keys.begin(); i != m_keys.end(); ++i)
		{
			EnvelopeKey* key = *i;
			int x = m_rcEnv.left + int(m_rcEnv.getWidth() * key->getT());
			int y = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(key->getValue(), m_minValue, m_maxValue));
			if (Rect(x - 2, y - 2, x + 3, y + 3).inside(pt))
			{
				m_selectedKey = key;
				break;
			}
		}
	}
	else if (event->getButton() == MbtRight)
	{
		m_selectedKey = new EnvelopeKey(
			float(pt.x - m_rcEnv.left) / m_rcEnv.getWidth(),
			float(pt.y - m_rcEnv.top) * (m_minValue - m_maxValue) / m_rcEnv.getHeight() + m_maxValue
		);

		insertKey(m_selectedKey);

		//Event changeEvent(this, m_selectedKey);
		//raiseEvent(&changeEvent);

		update();
	}

	if (m_selectedKey)
		setCapture();
}

void EnvelopeControl::eventButtonUp(MouseButtonUpEvent* event)
{
	if (m_selectedKey)
	{
		releaseCapture();
		m_selectedKey = 0;
	}
}

void EnvelopeControl::eventMouseMove(MouseMoveEvent* event)
{
	if (!m_selectedKey)
		return;

	Point pt = event->getPosition();

	float T = float(pt.x - m_rcEnv.left) / m_rcEnv.getWidth();
	float value = float(pt.y - m_rcEnv.top) * (m_minValue - m_maxValue) / m_rcEnv.getHeight() + m_maxValue;

	T = std::max< float >(0.0f, T);
	T = std::min< float >(1.0f, T);

	value = std::max< float >(m_minValue, value);
	value = std::min< float >(m_maxValue, value);

	if (m_selectedKey->getT() != T || m_selectedKey->getValue() != value)
	{
		if (!m_selectedKey->isFixedT())
			m_selectedKey->setT(T);

		if (!m_selectedKey->isFixedValue())
			m_selectedKey->setValue(value);

		EnvelopeContentChangeEvent changeEvent(this, m_selectedKey);
		raiseEvent(&changeEvent);
	}

	update();
}

void EnvelopeControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	Rect rcInner = getInnerRect();

	std::wstringstream mnv, mxv;
	mnv << m_minValue;
	mxv << m_maxValue;

	Size mne = canvas.getTextExtent(mnv.str());
	Size mxe = canvas.getTextExtent(mxv.str());

	int x = std::max< int >(mne.cx, mxe.cx);
	int y = std::max< int >(mne.cy, mxe.cy);
	m_rcEnv = Rect(
		rcInner.left + x + 8,
		rcInner.top + y / 2 + 4,
		rcInner.right - 4,
		rcInner.bottom - y / 2 - 4
	);

	canvas.setBackground(Color4ub(255, 255, 255));
	canvas.fillRect(rcInner);

	for (std::vector< Range >::const_iterator i = m_ranges.begin(); i != m_ranges.end(); ++i)
	{
		int y0 = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(i->limits[0], m_minValue, m_maxValue) + 0.5f);
		int y1 = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(i->limits[1], m_minValue, m_maxValue) + 0.5f);
		int y2 = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(i->limits[2], m_minValue, m_maxValue) + 0.5f);
		int y3 = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(i->limits[3], m_minValue, m_maxValue) + 0.5f);

		canvas.setForeground(i->color * Color4ub(255, 255, 255, 0));
		canvas.setBackground(i->color * Color4ub(255, 255, 255, 255));

		if (y0 > y1)
		{
			canvas.fillGradientRect(Rect(
				m_rcEnv.left,
				y0,
				m_rcEnv.right,
				y1
			));
		}

		if (y1 > y2)
		{
			canvas.fillRect(Rect(
				m_rcEnv.left,
				y1,
				m_rcEnv.right,
				y2
			));
		}

		if (y2 > y3)
		{
			canvas.fillGradientRect(Rect(
				m_rcEnv.left,
				y3,
				m_rcEnv.right,
				y2
			));
		}
	}

	canvas.drawRect(m_rcEnv.inflate(1, 1));

	canvas.setForeground(Color4ub(80, 80, 80));

	canvas.drawText(
		Point(m_rcEnv.left - mne.cx - 4, m_rcEnv.bottom - mne.cy / 2),
		mnv.str()
	);
	canvas.drawText(
		Point(m_rcEnv.left - mxe.cx - 4, m_rcEnv.top - mxe.cy / 2),
		mxv.str()
	);

	canvas.setForeground(Color4ub(120, 120, 120));

	int zero = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(0, m_minValue, m_maxValue));
	canvas.drawLine(m_rcEnv.left, zero, m_rcEnv.right, zero);

	if (!m_keys.empty())
	{
		if (m_selectedKey)
		{
			int sx = m_rcEnv.left + int(m_rcEnv.getWidth() * m_selectedKey->getT() + 0.5f);
			int sy = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(m_selectedKey->getValue(), m_minValue, m_maxValue) + 0.5f);
			canvas.setForeground(Color4ub(200, 200, 200));
			canvas.drawLine(m_rcEnv.left, sy, sx, sy);
			canvas.drawLine(sx, m_rcEnv.bottom, sx, sy);
		}

		canvas.setForeground(Color4ub(0, 0, 0));

		float dT = 1.0f / (rcInner.getSize().cx / 4.0f);

		int px = m_rcEnv.left;
		int py = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(m_evaluator->evaluate(m_keys, 0.0f), m_minValue, m_maxValue));
		for (float T = dT; T <= 1.0f; T += dT)
		{
			int sx = m_rcEnv.left + int(m_rcEnv.getWidth() * T + 0.5f);
			int sy = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(m_evaluator->evaluate(m_keys, T), m_minValue, m_maxValue) + 0.5f);
			canvas.drawLine(px, py, sx, sy);
			px = sx;
			py = sy;
		}

		canvas.setBackground(Color4ub(100, 100, 100));

		for (RefArray< EnvelopeKey >::iterator i = m_keys.begin(); i != m_keys.end(); ++i)
		{
			EnvelopeKey* key = *i;
			int sx = m_rcEnv.left + int(m_rcEnv.getWidth() * key->getT() + 0.5f);
			int sy = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(key->getValue(), m_minValue, m_maxValue) + 0.5f);
			canvas.fillRect(Rect(sx - 2, sy - 2, sx + 3, sy + 3));
		}
	}

	event->consume();
}

		}
	}
}
