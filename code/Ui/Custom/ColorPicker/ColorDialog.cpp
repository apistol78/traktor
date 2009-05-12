#include <sstream>
#include "Ui/Custom/ColorPicker/ColorDialog.h"
#include "Ui/Custom/ColorPicker/ColorGradientControl.h"
#include "Ui/Custom/ColorPicker/ColorSliderControl.h"
#include "Ui/Custom/ColorPicker/ColorControl.h"
#include "Ui/Custom/ColorPicker/ColorEvent.h"
#include "Ui/TableLayout.h"
#include "Ui/Static.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/FocusEvent.h"
#include "Core/Misc/StringUtils.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

struct ColorGradient : public ColorSliderControl::IGradient
{
	virtual Color get(int at) const
	{
		int rgb[] = { 255, 0, 0 };
		int i = 2;
		int d = 6;

		for (int y = 0; y < at; ++y)
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

		return Color(rgb[0], rgb[1], rgb[2], 255);
	}
};

struct AlphaGradient : public ColorSliderControl::IGradient
{
	Color color;

	virtual Color get(int at) const
	{
		return Color(color.r, color.g, color.b, at);
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ColorDialog", ColorDialog, ConfigDialog)

bool ColorDialog::create(Widget* parent, const std::wstring& text, int style, const Color& initialColor)
{
	if (!ConfigDialog::create(
		parent,
		text,
		500,
		400,
		style,
		gc_new< TableLayout >(L"*,*,*,*", L"*", 4, 4)
	))
		return false;

	m_gradientControl = gc_new< ColorGradientControl >();
	m_gradientControl->create(this, WsClientBorder, initialColor);
	m_gradientControl->addColorSelectEventHandler(createMethodHandler(this, &ColorDialog::eventGradientColorSelect));

	m_colorGradient = gc_new< ColorGradient >();

	m_sliderColorControl = gc_new< ColorSliderControl >();
	m_sliderColorControl->create(this, WsClientBorder, m_colorGradient);
	m_sliderColorControl->addColorSelectEventHandler(createMethodHandler(this, &ColorDialog::eventSliderColorSelect));

	if (style & WsAlpha)
	{
		m_alphaGradient = gc_new< AlphaGradient >();

		m_sliderAlphaControl = gc_new< ColorSliderControl >();
		m_sliderAlphaControl->create(this, WsClientBorder, m_alphaGradient);
		m_sliderAlphaControl->addColorSelectEventHandler(createMethodHandler(this, &ColorDialog::eventSliderAlphaSelect));
	}

	Ref< Container > container = gc_new< Container >();
	container->create(this, WsNone, gc_new< TableLayout >(L"*,100", L"*,*,*,*", 0, 4));

	Ref< Static > labelR = gc_new< Static >();
	labelR->create(container, L"R:");

	m_editColor[0] = gc_new< Edit >();
	m_editColor[0]->create(container, toString(initialColor.r), WsClientBorder, gc_new< NumericEditValidator >(false, 0, 255, 0));
	m_editColor[0]->addFocusEventHandler(createMethodHandler(this, &ColorDialog::eventEditFocus));

	Ref< Static > labelG = gc_new< Static >();
	labelG->create(container, L"G:");

	m_editColor[1] = gc_new< Edit >();
	m_editColor[1]->create(container, toString(initialColor.g), WsClientBorder, gc_new< NumericEditValidator >(false, 0, 255, 0));
	m_editColor[1]->addFocusEventHandler(createMethodHandler(this, &ColorDialog::eventEditFocus));

	Ref< Static > labelB = gc_new< Static >();
	labelB->create(container, L"B:");

	m_editColor[2] = gc_new< Edit >();
	m_editColor[2]->create(container, toString(initialColor.b), WsClientBorder, gc_new< NumericEditValidator >(false, 0, 255, 0));
	m_editColor[2]->addFocusEventHandler(createMethodHandler(this, &ColorDialog::eventEditFocus));

	if (style & WsAlpha)
	{
		Ref< Static > labelA = gc_new< Static >();
		labelA->create(container, L"A:");

		m_editColor[3] = gc_new< Edit >();
		m_editColor[3]->create(container, toString(initialColor.a), WsClientBorder, gc_new< NumericEditValidator >(false, 0, 255, 0));
		m_editColor[3]->addFocusEventHandler(createMethodHandler(this, &ColorDialog::eventEditFocus));
	}

	m_colorControl = gc_new< ColorControl >();
	m_colorControl->create(container, WsClientBorder);
	m_colorControl->setColor(initialColor);

	fit();
	return true;
}

Color ColorDialog::getColor() const
{
	return m_color;
}

void ColorDialog::updateControls()
{
	m_editColor[0]->setText(toString(m_color.r));
	m_editColor[1]->setText(toString(m_color.g));
	m_editColor[2]->setText(toString(m_color.b));

	if (m_editColor[3])
		m_editColor[3]->setText(toString(m_color.a));

	if (m_alphaGradient)
	{
		m_alphaGradient->color = m_color;
		m_sliderAlphaControl->updateGradient();
		m_sliderAlphaControl->update();
	}

	m_colorControl->setColor(m_color);
	m_colorControl->update();

	m_gradientControl->setColor(m_color, false);
	m_gradientControl->update();
}

void ColorDialog::eventGradientColorSelect(Event* event)
{
	Color color = checked_type_cast< ColorEvent* >(event)->getColor();
	
	m_color.r = color.r;
	m_color.g = color.g;
	m_color.b = color.b;

	m_editColor[0]->setText(toString(m_color.r));
	m_editColor[1]->setText(toString(m_color.g));
	m_editColor[2]->setText(toString(m_color.b));

	if (m_editColor[3])
		m_editColor[3]->setText(toString(m_color.a));

	if (m_alphaGradient)
	{
		m_alphaGradient->color = m_color;
		m_sliderAlphaControl->updateGradient();
		m_sliderAlphaControl->update();
	}

	m_colorControl->setColor(m_color);
	m_colorControl->update();
}

void ColorDialog::eventSliderColorSelect(Event* event)
{
	Color color = checked_type_cast< ColorEvent* >(event)->getColor();

	m_gradientControl->setColor(color, false);
	m_gradientControl->update();

	// Cycle color through gradient control as gradient colors are primary HSL colors.
	m_color = m_gradientControl->getColor();

	m_editColor[0]->setText(toString(m_color.r));
	m_editColor[1]->setText(toString(m_color.g));
	m_editColor[2]->setText(toString(m_color.b));

	if (m_editColor[3])
		m_editColor[3]->setText(toString(m_color.a));

	if (m_alphaGradient)
	{
		m_alphaGradient->color = m_color;
		m_sliderAlphaControl->updateGradient();
		m_sliderAlphaControl->update();
	}

	m_colorControl->setColor(m_color);
	m_colorControl->update();
}

void ColorDialog::eventSliderAlphaSelect(Event* event)
{
	Color alpha = checked_type_cast< ColorEvent* >(event)->getColor();

	m_color.a = alpha.a;

	if (m_editColor[3])
		m_editColor[3]->setText(toString(m_color.a));

	if (m_alphaGradient)
	{
		m_alphaGradient->color = m_color;
		m_sliderAlphaControl->updateGradient();
		m_sliderAlphaControl->update();
	}

	m_colorControl->setColor(m_color);
	m_colorControl->update();
}

void ColorDialog::eventEditFocus(Event* event)
{
	const FocusEvent* focusEvent = checked_type_cast< const FocusEvent* >(event);
	if (!focusEvent->lostFocus())
		return;

	int r = parseString< int >(m_editColor[0]->getText());
	int g = parseString< int >(m_editColor[1]->getText());
	int b = parseString< int >(m_editColor[2]->getText());
	int a = m_editColor[3] ? parseString< int >(m_editColor[3]->getText()) : 255;

	m_color.r = uint8_t(r);
	m_color.g = uint8_t(g);
	m_color.b = uint8_t(b);
	m_color.a = uint8_t(a);

	updateControls();
}

		}
	}
}
