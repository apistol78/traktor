/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/App/Win32/ErrorDialog.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Button.h"
#include "Ui/Container.h"
#include "Ui/Image.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/LogList/LogList.h"

// Resources
#include "Resources/Error.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.ErrorDialog", ErrorDialog, ui::Dialog)

bool ErrorDialog::create()
{
	const int32_t f = ui::dpi96(4);

	if (!ui::Dialog::create(
		nullptr,
		L"Error",
		ui::dpi96(700),
		ui::dpi96(450),
		ui::Dialog::WsDefaultResizable | ui::Dialog::WsCenterDesktop,
		new ui::TableLayout(L"*,100%", L"100%", 0, 0)
	))
		return false;

	Ref< ui::Image > imageError = new ui::Image();
	imageError->create(this, ui::Bitmap::load(c_ResourceError, sizeof(c_ResourceError), L"image"), ui::Image::WsTransparent);

	Ref< ui::Container > container = new ui::Container();
	container->create(this, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", f, f));

	Ref< ui::Container > containerText = new ui::Container();
	containerText->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, f));

	Ref< ui::Static > staticText = new ui::Static();
	staticText->create(containerText, L"Unfortunately there has been an error");

	m_listLog = new ui::LogList();
	m_listLog->create(containerText, ui::WsNone, 0);

	Ref< ui::Container > containerButtons = new ui::Container();
	containerButtons->create(container, ui::WsNone, new ui::TableLayout(L"100%,*,*", L"*", 0, f));

	Ref< ui::Static > staticDummy = new ui::Static();
	staticDummy->create(containerButtons, L"");

	Ref< ui::Button > buttonCopy = new ui::Button();
	buttonCopy->create(containerButtons, L"Copy to clipboard");
	buttonCopy->addEventHandler< ui::ButtonClickEvent >(this, &ErrorDialog::eventButtonClickCopy);

	Ref< ui::Button > buttonQuit = new ui::Button();
	buttonQuit->create(containerButtons, L"Quit");
	buttonQuit->addEventHandler< ui::ButtonClickEvent >(this, &ErrorDialog::eventButtonClickQuit);

	return true;
}

void ErrorDialog::addErrorString(const std::wstring& errorString)
{
	m_listLog->add(0, ui::LogList::LvInfo, errorString);
}

void ErrorDialog::eventButtonClickCopy(ui::ButtonClickEvent* event)
{
	m_listLog->copyLog();
}

void ErrorDialog::eventButtonClickQuit(ui::ButtonClickEvent* event)
{
	endModal(ui::DialogResult::Ok);
}

}
