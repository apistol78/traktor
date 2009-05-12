#include <sstream>
#include "Ui/Custom/Envelope/EnvelopeControl.h"
#include "Ui/Custom/Envelope/EnvelopeEvaluator.h"
#include "Ui/Custom/Envelope/EnvelopeKey.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"

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
	return (value - minValue) / (maxValue - minValue);
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.EnvelopeControl", EnvelopeControl, Widget)

bool EnvelopeControl::create(Widget* parent, EnvelopeEvaluator* evaluator, float minValue, float maxValue, int style)
{
	if (!Widget::create(parent, style))
		return false;

	addButtonDownEventHandler(createMethodHandler(this, &EnvelopeControl::eventButtonDown));
	addButtonUpEventHandler(createMethodHandler(this, &EnvelopeControl::eventButtonUp));
	addMouseMoveEventHandler(createMethodHandler(this, &EnvelopeControl::eventMouseMove));
	addPaintEventHandler(createMethodHandler(this, &EnvelopeControl::eventPaint));

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

void EnvelopeControl::addChangeEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiContentChange, eventHandler);
}

void EnvelopeControl::eventButtonDown(Event* e)
{
	MouseEvent* m = static_cast< MouseEvent* >(e);
	Point pt = m->getPosition();

	m_selectedKey = 0;

	if (m->getButton() == MouseEvent::BtLeft)
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
	else if (m->getButton() == MouseEvent::BtRight)
	{
		m_selectedKey = gc_new< EnvelopeKey >(
			float(pt.x - m_rcEnv.left) / m_rcEnv.getWidth(),
			float(pt.y - m_rcEnv.top) * (m_minValue - m_maxValue) / m_rcEnv.getHeight() + m_maxValue
		);

		insertKey(m_selectedKey);

		Event changeEvent(this, m_selectedKey);
		raiseEvent(EiContentChange, &changeEvent);

		update();
	}

	if (m_selectedKey)
		setCapture();
}

void EnvelopeControl::eventButtonUp(Event* e)
{
	if (m_selectedKey)
	{
		releaseCapture();
		m_selectedKey = 0;
	}
}

void EnvelopeControl::eventMouseMove(Event* e)
{
	if (!m_selectedKey)
		return;

	MouseEvent* m = static_cast< MouseEvent* >(e);
	Point pt = m->getPosition();

	float T = float(pt.x - m_rcEnv.left) / m_rcEnv.getWidth();
	float value = float(pt.y - m_rcEnv.top) * (m_minValue - m_maxValue) / m_rcEnv.getHeight() + m_maxValue;

	T = std::max< float >(0.0f, T);
	T = std::min< float >(1.0f, T);

	value = std::max< float >(m_minValue, value);
	value = std::min< float >(m_maxValue, value);

	if (m_selectedKey->getT() != T || m_selectedKey->getValue() != value)
	{
		m_selectedKey->setT(T);
		m_selectedKey->setValue(value);

		Event changeEvent(this, m_selectedKey);
		raiseEvent(EiContentChange, &changeEvent);
	}

	update();
}

void EnvelopeControl::eventPaint(Event* e)
{
	PaintEvent* p = static_cast< PaintEvent* >(e);
	Canvas& canvas = p->getCanvas();

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

	canvas.setForeground(Color(180, 180, 180));
	canvas.setBackground(Color(255, 255, 255));
	canvas.fillRect(rcInner);
	canvas.drawRect(m_rcEnv.inflate(1, 1));

	canvas.setForeground(Color(80, 80, 80));

	canvas.drawText(
		Point(m_rcEnv.left - mne.cx - 4, m_rcEnv.bottom - mne.cy / 2),
		mnv.str()
	);
	canvas.drawText(
		Point(m_rcEnv.left - mxe.cx - 4, m_rcEnv.top - mxe.cy / 2),
		mxv.str()
	);

	canvas.setForeground(Color(120, 120, 120));

	int zero = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(0, m_minValue, m_maxValue));
	canvas.drawLine(m_rcEnv.left, zero, m_rcEnv.right, zero);

	if (!m_keys.empty())
	{
		if (m_selectedKey)
		{
			int sx = m_rcEnv.left + int(m_rcEnv.getWidth() * m_selectedKey->getT());
			int sy = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(m_selectedKey->getValue(), m_minValue, m_maxValue));
			canvas.setForeground(Color(200, 200, 200));
			canvas.drawLine(m_rcEnv.left, sy, sx, sy);
		}

		canvas.setForeground(Color(0, 0, 0));

		const float dT = 1.0f / 100.0f;
		int px = m_rcEnv.left;
		int py = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(m_evaluator->evaluate(m_keys, 0.0f), m_minValue, m_maxValue));
		for (float T = dT; T <= 1.0f; T += dT)
		{
			int sx = m_rcEnv.left + int(m_rcEnv.getWidth() * T);
			int sy = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(m_evaluator->evaluate(m_keys, T), m_minValue, m_maxValue));
			canvas.drawLine(px, py, sx, sy);
			px = sx;
			py = sy;
		}

		canvas.setBackground(Color(0, 0, 0));

		for (RefArray< EnvelopeKey >::iterator i = m_keys.begin(); i != m_keys.end(); ++i)
		{
			EnvelopeKey* key = *i;
			int sx = m_rcEnv.left + int(m_rcEnv.getWidth() * key->getT());
			int sy = m_rcEnv.bottom - int(m_rcEnv.getHeight() * scaleValue(key->getValue(), m_minValue, m_maxValue));
			canvas.fillRect(Rect(sx - 2, sy - 2, sx + 3, sy + 3));
		}
	}

	p->consume();
}

		}
	}
}
