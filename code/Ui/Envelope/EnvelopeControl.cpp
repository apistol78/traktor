/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/Envelope/EnvelopeContentChangeEvent.h"
#include "Ui/Envelope/EnvelopeControl.h"
#include "Ui/Envelope/EnvelopeEvaluator.h"
#include "Ui/Envelope/EnvelopeKey.h"

namespace traktor::ui
{
	namespace
	{

float scaleValue(float value, float minValue, float maxValue)
{
	return clamp((value - minValue) / (maxValue - minValue), minValue, maxValue);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EnvelopeControl", EnvelopeControl, Widget)

bool EnvelopeControl::create(Widget* parent, EnvelopeEvaluator* evaluator, float minValue, float maxValue, uint32_t style)
{
	if (!Widget::create(parent, style | WsFocus))
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
	for (auto it = m_keys.begin(); it != m_keys.end(); ++it)
	{
		if (key->getT() < (*it)->getT())
		{
			m_keys.insert(it, key);
			return;
		}
	}
	m_keys.push_back(key);
}

const RefArray< EnvelopeKey >& EnvelopeControl::getKeys() const
{
	return m_keys;
}

void EnvelopeControl::eventButtonDown(MouseButtonDownEvent* event)
{
	const Point pt = event->getPosition();

	m_selectedKey = nullptr;

	if (event->getButton() == MbtLeft)
	{
		const int32_t sx = pixel(4_ut);
		const int32_t sy = pixel(4_ut);
		
		for (auto key : m_keys)
		{
			const int32_t x = m_rcEnv.left + (int32_t)(m_rcEnv.getWidth() * key->getT());
			const int32_t y = m_rcEnv.bottom - (int32_t)(m_rcEnv.getHeight() * scaleValue(key->getValue(), m_minValue, m_maxValue));
			if (Rect(x - sx, y - sy, x + sx, y + sy).inside(pt))
			{
				m_selectedKey = key;
				break;
			}
		}

		update();
	}
	else if (event->getButton() == MbtRight)
	{
		m_selectedKey = new EnvelopeKey(
			(float)(pt.x - m_rcEnv.left) / m_rcEnv.getWidth(),
			(float)(pt.y - m_rcEnv.top) * (m_minValue - m_maxValue) / m_rcEnv.getHeight() + m_maxValue
		);

		insertKey(m_selectedKey);
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
		m_selectedKey = nullptr;
	}
}

void EnvelopeControl::eventMouseMove(MouseMoveEvent* event)
{
	if (!m_selectedKey)
		return;

	const Point pt = event->getPosition();

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
	const StyleSheet* ss = getStyleSheet();

	const Rect rcInner = getInnerRect();

	const std::wstring mnv = str(L"%.1f", m_minValue);
	const std::wstring mxv = str(L"%.1f", m_maxValue);

	const Size mne = canvas.getFontMetric().getExtent(mnv);
	const Size mxe = canvas.getFontMetric().getExtent(mxv);

	const int32_t x = std::max< int32_t >(mne.cx, mxe.cx);
	const int32_t y = std::max< int32_t >(mne.cy, mxe.cy);
	m_rcEnv = Rect(
		rcInner.left + x + 8,
		rcInner.top + y / 2 + 4,
		rcInner.right - 4,
		rcInner.bottom - y / 2 - 4
	);

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.setForeground(ss->getColor(this, L"color"));

	canvas.fillRect(rcInner);
	canvas.drawRect(m_rcEnv.inflate(1, 1));

	canvas.drawText(
		Point(m_rcEnv.left - mne.cx - 4, m_rcEnv.bottom - mne.cy / 2),
		mnv
	);
	canvas.drawText(
		Point(m_rcEnv.left - mxe.cx - 4, m_rcEnv.top - mxe.cy / 2),
		mxv
	);

	const int32_t zero = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(0, m_minValue, m_maxValue));
	canvas.drawLine(m_rcEnv.left, zero, m_rcEnv.right, zero);

	if (!m_keys.empty())
	{
		canvas.setForeground(ss->getColor(this, L"color-line"));

		const float dT = 1.0f / (rcInner.getSize().cx / 8.0f);

		std::vector< Point > points;
		for (float T = 0.0f; T <= 1.0f + dT; T += dT)
		{
			const float cT = std::min< float >(T, 1.0f);
			const int32_t sx = m_rcEnv.left + int32_t(m_rcEnv.getWidth() * cT);
			const int32_t sy = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(m_evaluator->evaluate(m_keys, cT), m_minValue, m_maxValue));
			points.push_back({ sx, sy });
		}
		canvas.drawLines(points);

		canvas.setBackground(ss->getColor(this, L"color-key"));
		for (auto key : m_keys)
		{
			const int32_t sx = m_rcEnv.left + int32_t(m_rcEnv.getWidth() * key->getT());
			const int32_t sy = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(key->getValue(), m_minValue, m_maxValue));
			canvas.fillCircle(Point(sx, sy), (key == m_selectedKey) ? 5.0f : 3.0f);
		}
	}

	event->consume();
}

}
