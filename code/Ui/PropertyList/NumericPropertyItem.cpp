/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cmath>
#include <sstream>
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/StyleBitmap.h"
#include "Ui/PropertyList/NumericPropertyItem.h"
#include "Ui/PropertyList/PropertyList.h"

namespace traktor::ui
{
	namespace
	{

double trunc(double value)
{
	return (double)((long)value);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NumericPropertyItem", NumericPropertyItem, PropertyItem)

NumericPropertyItem::NumericPropertyItem(const std::wstring& text, double value, double limitMin, double limitMax, bool floatPoint, bool hex, Representation representation)
:	PropertyItem(text)
,	m_value(value)
,	m_limitMin(limitMin)
,	m_limitMax(limitMax)
,	m_floatPoint(floatPoint)
,	m_hex(hex)
,	m_representation(representation)
,	m_mouseAdjust(false)
{
}

void NumericPropertyItem::setValue(double value)
{
	m_value = value;
}

double NumericPropertyItem::getValue() const
{
	return m_floatPoint ? m_value : trunc(m_value);
}

void NumericPropertyItem::setLimitMin(double limitMin)
{
	m_limitMin = limitMin;
	m_value = std::max< double >(m_value, m_limitMin);
}

double NumericPropertyItem::getLimitMin() const
{
	return m_limitMin;
}

void NumericPropertyItem::setLimitMax(double limitMax)
{
	m_limitMax = limitMax;
	m_value = std::min< double >(m_value, m_limitMax);
}

double NumericPropertyItem::getLimitMax() const
{
	return m_limitMax;
}

void NumericPropertyItem::setLimit(double limitMin, double limitMax)
{
	setLimitMin(limitMin);
	setLimitMax(limitMax);
}

void NumericPropertyItem::createInPlaceControls(PropertyList* parent)
{
	T_ASSERT(!m_editor);

	const float limitMin = (m_representation == RpPercent) ? m_limitMin * 100.0f : m_limitMin;
	const float limitMax = (m_representation == RpPercent) ? m_limitMax * 100.0f : m_limitMax;

	m_editor = new Edit();
	m_editor->create(
		parent,
		L"",
		WsWantAllInput,
		m_hex ? nullptr : new NumericEditValidator(
			m_floatPoint,
			limitMin,
			limitMax
		)
	);
	m_editor->setVisible(false);
	m_editor->addEventHandler< FocusEvent >(this, &NumericPropertyItem::eventEditFocus);
	m_editor->addEventHandler< KeyDownEvent >(this, &NumericPropertyItem::eventEditKeyDownEvent);

	m_upDown[0] = new StyleBitmap(L"UI.UpDown0");
	m_upDown[1] = new StyleBitmap(L"UI.UpDown1");
	m_upDown[2] = new StyleBitmap(L"UI.UpDown2");
	m_upDown[3] = new StyleBitmap(L"UI.UpDown3");
}

void NumericPropertyItem::destroyInPlaceControls()
{
	safeDestroy(m_editor);
}

void NumericPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	if (m_editor)
	{
		const Rect rcEditor(rc.left, rc.top, rc.right - rc.getHeight(), rc.bottom);
		outChildRects.push_back(WidgetRect(m_editor, rcEditor));
	}
}

void NumericPropertyItem::mouseButtonDown(MouseButtonDownEvent* event)
{
	if (event->getPosition().x <= m_editor->getRect().right)
	{
		std::wstringstream ss;

		const double value = getValue();
		if (m_hex)
			ss << std::hex << uint32_t(value);
		else
		{
			if (m_representation == RpAngle)
				ss << rad2deg(value);
			else if (m_representation == RpPercent)
				ss << (value * 100.0f);
			else
				ss << value;
		}

		m_editor->setText(ss.str());
		m_editor->setVisible(true);
		m_editor->setFocus();
		m_editor->selectAll();
	}
	else
	{
		m_mouseAdjust = true;
		m_mouseLastPosition = event->getPosition();
	}
}

void NumericPropertyItem::mouseButtonUp(MouseButtonUpEvent* event)
{
	m_mouseAdjust = false;
}

void NumericPropertyItem::mouseMove(MouseMoveEvent* event)
{
	if (m_mouseAdjust)
	{
		m_value += -(event->getPosition().y - m_mouseLastPosition.y) / 20.0f;
		m_value = std::max(m_value, m_limitMin);
		m_value = std::min(m_value, m_limitMax);
		m_mouseLastPosition = event->getPosition();
		notifyUpdate();
		notifyChange();
	}
}

void NumericPropertyItem::paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc)
{
	std::wstringstream ss;

	double value = getValue();
	if (m_hex)
		ss << L"0x" << std::hex << uint32_t(value);
	else
	{
		if (m_representation == RpAngle)
			ss << rad2deg(value) << L" \xb0";
		else if (m_representation == RpAnglesPerSecond)
			ss << rad2deg(value) << L" \xb0/s";
		else if (m_representation == RpMetres)
			ss << value << L" m";
		else if (m_representation == RpMetresPerSecond)
			ss << value << L" m/s";
		else if (m_representation == RpKilograms)
			ss << value << L" kg";
		else if (m_representation == RpPercent)
			ss << (value * 100.0f) << L" %";
		else if (m_representation == RpHerz)
			ss << value << L" Hz";
		else if (m_representation == RpEV)
			ss << value << L" EV";
		else if (m_representation == RpNewton)
			ss << value << L" N";
		else if (m_representation == RpNewtonSecond)
			ss << value << L" Ns";
		else if (m_representation == RpSeconds)
			ss << value << L" s";
		else
			ss << value;
	}

	if (m_representation == RpDecibel)
		ss << L" dB";

	canvas.drawText(rc.inflate(-2, 0), ss.str(), AnLeft, AnCenter);

	int32_t index = 0;
	if (value > m_limitMin)
		index |= 1;
	if (value < m_limitMax)
		index |= 2;

	auto upDown = m_upDown[index];
	auto upDownSize = upDown->getSize(parent);

	const int32_t x = rc.right - upDownSize.cx - parent->pixel(2_ut);
	const int32_t y = rc.top + (rc.getHeight() - upDownSize.cy) / 2;

	canvas.drawBitmap(
		Point(x, y),
		Point(0, 0),
		upDown->getSize(parent),
		upDown,
		BlendMode::Alpha
	);
}

bool NumericPropertyItem::copy()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (clipboard)
		return clipboard->setText(toString(m_value));
	else
		return false;
}

bool NumericPropertyItem::paste()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	const double value = parseString< double >(clipboard->getText());
	if (!std::isnan(value))
	{
		m_value = value;
		return true;
	}
	else
		return false;
}

void NumericPropertyItem::eventEditFocus(FocusEvent* event)
{
	if (event->lostFocus() && m_editor->isVisible(false))
	{
		std::wstringstream ss(m_editor->getText());

		if (m_hex)
		{
			uint32_t value;
			ss >> std::hex >> value;
			m_value = double(value);
		}
		else
		{
			ss >> m_value;
			if (m_representation == RpAngle || m_representation == RpAnglesPerSecond)
				m_value = deg2rad(m_value);
			else if (m_representation == RpPercent)
				m_value = m_value / 100.0f;
		}

		m_editor->setVisible(false);

		notifyChange();
	}
}

void NumericPropertyItem::eventEditKeyDownEvent(KeyDownEvent* event)
{
	if (event->getVirtualKey() == ui::VkReturn)
	{
		std::wstringstream ss(m_editor->getText());

		if (m_hex)
		{
			uint32_t value;
			ss >> std::hex >> value;
			m_value = double(value);
		}
		else
		{
			ss >> m_value;
			if (m_representation == RpAngle || m_representation == RpAnglesPerSecond)
				m_value = deg2rad(m_value);
			else if (m_representation == RpPercent)
				m_value = m_value / 100.0f;
		}

		m_editor->setVisible(false);

		notifyChange();
	}
	else if (event->getVirtualKey() == ui::VkEscape)
		m_editor->setVisible(false);
}

}
