/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/RadioButton.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.RadioButton", RadioButton, Widget)

bool RadioButton::create(Widget* parent, const std::wstring& text, bool checked)
{
	if (!Widget::create(parent))
		return false;

	return true;
}

void RadioButton::setChecked(bool checked)
{
	//static_cast< IRadioButton* >(m_widget)->setChecked(checked);
}

bool RadioButton::isChecked() const
{
	//return static_cast< IRadioButton* >(m_widget)->isChecked();
	return false;
}

}
