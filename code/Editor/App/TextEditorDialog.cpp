/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/App/TextEditorDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/StyleBitmap.h"
#include "Ui/RichEdit/RichEdit.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.TextEditorDialog", TextEditorDialog, ui::ConfigDialog)

bool TextEditorDialog::create(ui::Widget* parent, const std::wstring& initialText)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"TEXT_EDIT"),
		500_ut,
		400_ut,
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::FloodLayout()
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	m_edit = new ui::RichEdit();
	if (!m_edit->create(this, initialText, ui::WsDoubleBuffer))
		return false;

	m_edit->setFont(ui::Font(L"Courier New", 14));

	update();

	return true;
}

std::wstring TextEditorDialog::getText() const
{
	return m_edit->getText();
}

	}
}
