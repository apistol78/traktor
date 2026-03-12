/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/App/Splash.h"

#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/FloodLayout.h"
#include "Ui/Image.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.Splash", Splash, ui::Dialog)

bool Splash::create()
{
	if (!ui::Dialog::create(nullptr, L"", 0_ut, 0_ut, ui::WsTop | ui::Dialog::WsCenterDesktop, new ui::FloodLayout()))
		return false;

	// Since no stylesheet is currently loaded we hardcode the splash image and append to default sheet.
	Ref< ui::StyleSheet > ss = new ui::StyleSheet();
	ss->setValue(L"Editor.Splash", L"$(TRAKTOR_HOME)/resources/runtime/themes/Shared/Images/Editor/Splash.svg");
	ui::Application::getInstance()->appendStyleSheet(ss);

	Ref< ui::StyleBitmap > splash = new ui::StyleBitmap(L"Editor.Splash");

	Ref< ui::Image > image = new ui::Image();
	image->create(this, splash, ui::Image::WsScale);

	const ui::Size sz = splash->getSize(this);
	setRect({ 0, 0, sz.cx, sz.cy });

	show();

	update(nullptr, true);
	return true;
}

}
