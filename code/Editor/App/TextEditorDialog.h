/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/ConfigDialog.h"

namespace traktor::ui
{

class RichEdit;

}

namespace traktor::editor
{

class IEditor;

class TextEditorDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	explicit TextEditorDialog(IEditor* editor);

	bool create(ui::Widget* parent, const std::wstring& initialText);

	virtual std::wstring getText() const override final;

private:
	IEditor* m_editor;
	Ref< ui::RichEdit > m_edit;
};

}
