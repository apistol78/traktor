/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <sstream>
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/ColorPicker/ColorDialog.h"
#include "Ui/ColorPicker/ColorGradientControl.h"
#include "Ui/ColorPicker/ColorSliderControl.h"
#include "Ui/ColorPicker/ColorControl.h"
#include "Ui/ColorPicker/ColorEvent.h"
#include "Ui/ColorPicker/ColorUtilities.h"

namespace traktor::ui
{

struct ColorGradient : public ColorSliderControl::IGradient
{
	virtual Color4ub get(int32_t at) const
	{
		int32_t rgb[] = { 255, 0, 0 };
		int32_t i = 2;
		int32_t d = 6;

		for (int32_t y = 0; y < at; ++y)
		{
			rgb[i] += d;
			if ((d < 0 && rgb[i] <= 0) || (d > 0 && rgb[i] >= 255))
			{
				rgb[i] = std::max(  0, rgb[i]);
				rgb[i] = std::min(255, rgb[i]);

				i = (i + 1) % 3;
				d = -d;
			}
		}

		return Color4ub(rgb[0], rgb[1], rgb[2], 255);
	}
};

struct AlphaGradient : public ColorSliderControl::IGradient
{
	Color4ub color = Color4ub(0, 0, 0, 255);

	virtual Color4ub get(int32_t at) const
	{
		return Color4ub(color.r, color.g, color.b, at);
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ColorDialog", ColorDialog, ConfigDialog)

bool ColorDialog::create(Widget* parent, const std::wstring& text, uint32_t style, const Color4f& initialColor)
{
	if (!ConfigDialog::create(
		parent,
		text,
		500_ut,
		400_ut,
		style,
		new TableLayout(L"*,*,*,*", L"*", 4_ut, 4_ut)
	))
		return false;

	m_gradientControl = new ColorGradientControl();
	m_gradientControl->create(this, WsDoubleBuffer | WsTabStop);
	m_gradientControl->addEventHandler< ColorEvent >(this, &ColorDialog::eventGradientColorSelect);

	m_colorGradient = new ColorGradient();

	m_sliderColorControl = new ColorSliderControl();
	m_sliderColorControl->create(this, WsDoubleBuffer | WsTabStop, m_colorGradient);
	m_sliderColorControl->addEventHandler< ColorEvent >(this, &ColorDialog::eventSliderColorSelect);

	if (style & WsAlpha)
	{
		m_alphaGradient = new AlphaGradient();

		m_sliderAlphaControl = new ColorSliderControl();
		m_sliderAlphaControl->create(this, WsDoubleBuffer | WsTabStop, m_alphaGradient);
		m_sliderAlphaControl->addEventHandler< ColorEvent >(this, &ColorDialog::eventSliderAlphaSelect);
	}

	Ref< Container > container = new Container();
	container->create(this, WsNone, new TableLayout(L"*,100", L"*,*,*,*,*", 0_ut, 4_ut));

	Ref< Static > labelR = new Static();
	labelR->create(container, L"R:");

	m_editColor[0] = new Edit();
	m_editColor[0]->create(container, L"0", WsTabStop, new NumericEditValidator(false, 0, 255, 0));
	m_editColor[0]->addEventHandler< FocusEvent >(this, &ColorDialog::eventEditFocus);

	Ref< Static > labelG = new Static();
	labelG->create(container, L"G:");

	m_editColor[1] = new Edit();
	m_editColor[1]->create(container, L"0", WsTabStop, new NumericEditValidator(false, 0, 255, 0));
	m_editColor[1]->addEventHandler< FocusEvent >(this, &ColorDialog::eventEditFocus);

	Ref< Static > labelB = new Static();
	labelB->create(container, L"B:");

	m_editColor[2] = new Edit();
	m_editColor[2]->create(container, L"0", WsTabStop, new NumericEditValidator(false, 0, 255, 0));
	m_editColor[2]->addEventHandler< FocusEvent >(this, &ColorDialog::eventEditFocus);

	if (style & WsAlpha)
	{
		Ref< Static > labelA = new Static();
		labelA->create(container, L"A:");

		m_editColor[3] = new Edit();
		m_editColor[3]->create(container, L"255", WsTabStop, new NumericEditValidator(false, 0, 255, 0));
		m_editColor[3]->addEventHandler< FocusEvent >(this, &ColorDialog::eventEditFocus);
	}

	if (style & WsHDR)
	{
		Ref< Static > labelEV = new Static();
		labelEV->create(container, L"EV:");

		m_editColor[4] = new Edit();
		m_editColor[4]->create(container, L"0", WsTabStop, new NumericEditValidator(false, -10, 10, 0));
		m_editColor[4]->addEventHandler< FocusEvent >(this, &ColorDialog::eventEditFocus);
	}

	m_colorControl = new ColorControl();
	m_colorControl->create(container, WsTabStop);

	setColor(initialColor);

	fit(Container::Both);
	return true;
}

void ColorDialog::setColor(const Color4f& color)
{
	m_color = color;
	m_color.setEV(0.0_simd);

	m_ev = color.getEV();

	const Color4ub cl = m_color.saturated().toColor4ub();

	m_editColor[0]->setText(toString< int32_t >(cl.r));
	m_editColor[1]->setText(toString< int32_t >(cl.g));
	m_editColor[2]->setText(toString< int32_t >(cl.b));

	if (m_editColor[3])
		m_editColor[3]->setText(toString< int32_t >(cl.a));

	if (m_editColor[4])
		m_editColor[4]->setText(toString< int32_t >(int32_t(m_ev + 0.5_simd)));

	// Figure out primary color.
	float hsv[3];
	RGBtoHSV(m_color, hsv);
	hsv[1] = 1.0f;
	hsv[2] = 1.0f;
	Color4f tmp;
	HSVtoRGB(hsv, tmp);
	Color4ub primaryColor = tmp.toColor4ub();

	// Update cursors in widgets.
	m_gradientControl->setPrimaryColor(primaryColor);
	m_gradientControl->setCursorColor(cl);
	m_gradientControl->update();

	if (m_alphaGradient)
	{
		m_alphaGradient->color = cl;
		m_sliderAlphaControl->updateGradient();
		m_sliderAlphaControl->update();
	}

	m_colorControl->setColor(cl);
	m_colorControl->update();
}

Color4f ColorDialog::getColor() const
{
	Color4f cl = m_color;
	cl.setEV(m_ev);
	return cl;
}

void ColorDialog::updateTextControls()
{
	Color4f cl = m_color;
	cl.setEV(0.0_simd);

	const Color4ub club = cl.saturated().toColor4ub();

	m_editColor[0]->setText(toString< int32_t >(club.r));
	m_editColor[1]->setText(toString< int32_t >(club.g));
	m_editColor[2]->setText(toString< int32_t >(club.b));

	if (m_editColor[3])
		m_editColor[3]->setText(toString< int32_t >(club.a));

	if (m_editColor[4])
		m_editColor[4]->setText(toString< int32_t >(int32_t(m_color.getEV() + 0.5f)));
}

void ColorDialog::eventGradientColorSelect(ColorEvent* event)
{
	// Color selected from large gradient.
	Color4ub color = event->getColor();
	color.a = m_editColor[3] ? parseString< int32_t >(m_editColor[3]->getText()) : 255;

	m_color = Color4f::fromColor4ub(color);

	if (m_editColor[4])
	{
		const float ev = (float)parseString< int32_t >(m_editColor[4]->getText());
		m_color.setEV(Scalar(ev));
	}

	if (m_alphaGradient)
	{
		m_alphaGradient->color = color;
		m_sliderAlphaControl->updateGradient();
		m_sliderAlphaControl->update();
	}

	m_colorControl->setColor(color);
	m_colorControl->update();

	updateTextControls();
}

void ColorDialog::eventSliderColorSelect(ColorEvent* event)
{
	// Color selected from primary color slider.
	const Color4ub primaryColor = event->getColor();

	m_gradientControl->setPrimaryColor(primaryColor);
	m_gradientControl->update();

	Color4ub color = m_gradientControl->getColor();
	color.a = m_editColor[3] ? parseString< int32_t >(m_editColor[3]->getText()) : 255;

	m_color = Color4f::fromColor4ub(color);

	if (m_editColor[4])
	{
		const float ev = (float)parseString< int32_t >(m_editColor[4]->getText());
		m_color.setEV(Scalar(ev));
	}

	if (m_alphaGradient)
	{
		m_alphaGradient->color = color;
		m_sliderAlphaControl->updateGradient();
		m_sliderAlphaControl->update();
	}

	m_colorControl->setColor(color);
	m_colorControl->update();

	updateTextControls();
}

void ColorDialog::eventSliderAlphaSelect(ColorEvent* event)
{
	// Alpha selected from alpha slider.
	const Color4ub alpha = event->getColor();

	Color4ub color = m_gradientControl->getColor();
	color.a = alpha.a;

	m_color = Color4f::fromColor4ub(color);

	if (m_editColor[4])
	{
		const float ev = (float)parseString< int32_t >(m_editColor[4]->getText());
		m_color.setEV(Scalar(ev));
	}

	m_colorControl->setColor(color);
	m_colorControl->update();

	updateTextControls();
}

void ColorDialog::eventEditFocus(FocusEvent* event)
{
	if (!event->lostFocus())
		return;

	const int32_t r = parseString< int32_t >(m_editColor[0]->getText());
	const int32_t g = parseString< int32_t >(m_editColor[1]->getText());
	const int32_t b = parseString< int32_t >(m_editColor[2]->getText());
	const int32_t a = m_editColor[3] ? parseString< int32_t >(m_editColor[3]->getText()) : 255;
	const float ev = m_editColor[4] ? (float)parseString< int32_t >(m_editColor[4]->getText()) : 0.0f;

	const Color4ub color(r, g, b, a);

	// Figure out primary color.
	float hsv[3];
	RGBtoHSV(Color4f::fromColor4ub(color), hsv);
	hsv[1] = 1.0f;
	hsv[2] = 1.0f;
	Color4f tmp;
	HSVtoRGB(hsv, tmp);
	Color4ub primaryColor = tmp.toColor4ub();

	// Update cursors in widgets.
	m_gradientControl->setPrimaryColor(primaryColor);
	m_gradientControl->setCursorColor(color);
	m_gradientControl->update();

	m_color = Color4f::fromColor4ub(color);
	m_ev = Scalar(ev);

	if (m_alphaGradient)
	{
		m_alphaGradient->color = color;
		m_sliderAlphaControl->updateGradient();
		m_sliderAlphaControl->update();
	}

	m_colorControl->setColor(color);
	m_colorControl->update();
}

}
