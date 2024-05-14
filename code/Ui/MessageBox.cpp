/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/Image.h"
#include "Ui/MessageBox.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MessageBox", MessageBox, ConfigDialog)

bool MessageBox::create(Widget* parent, const std::wstring& message, const std::wstring& caption, uint32_t style)
{
	uint32_t dialogStyle = WsCenterParent | WsSystemBox | WsCloseBox | WsCaption;
	if (style & MbYesNo)
		dialogStyle |= ConfigDialog::WsYesNoButtons;

	const bool haveIcon = (bool)((style & (MbIconExclamation | MbIconHand | MbIconError | MbIconQuestion | MbIconInformation)) != 0);

	if (!ConfigDialog::create(
		parent,
		caption,
		200_ut, 100_ut,
		dialogStyle,
		new TableLayout(haveIcon ? L"*,*" : L"*", L"100%", 16_ut, 16_ut)
	))
		return false;

	if (haveIcon)
	{
		Ref< IBitmap > icon;

		if ((style & MbIconExclamation) != 0)
			icon = new StyleBitmap(L"UI.IconExclamation");
		else if ((style & MbIconHand) != 0)
			icon = new StyleBitmap(L"UI.IconHand");
		else if ((style & MbIconError) != 0)
			icon = new StyleBitmap(L"UI.IconError");
		else if ((style & MbIconQuestion) != 0)
			icon = new StyleBitmap(L"UI.IconQuestion");
		else if ((style & MbIconInformation) != 0)
			icon = new StyleBitmap(L"UI.IconInformation");

		T_ASSERT(icon != nullptr);

		Ref< Image > image = new Image();
		image->create(this, icon, Image::WsTransparent);
		image->setVerticalAlign(AnCenter);
	}

	Ref< Static > staticMessage = new Static();
	staticMessage->create(this, message);
	staticMessage->setVerticalAlign(AnCenter);

	fit(Container::Both);
	return true;
}

DialogResult MessageBox::show(Widget* parent, const std::wstring& message, const std::wstring& caption, uint32_t style)
{
	MessageBox mb;
	if (!mb.create(parent, message, caption, style))
		return DialogResult::Cancel;

	const DialogResult result = mb.showModal();

	mb.destroy();
	return result;
}

DialogResult MessageBox::show(const std::wstring& message, const std::wstring& caption, uint32_t style)
{
	return show(nullptr, message, caption, style);
}

}
