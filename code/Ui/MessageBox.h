/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/ConfigDialog.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

// Remove MS definition.
#if defined(MessageBox)
#	undef MessageBox
#endif

/*! Message box.
 * \ingroup UI
 */
class T_DLLCLASS MessageBox : public ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& message, const std::wstring& caption, uint32_t style);

	static DialogResult show(Widget* parent, const std::wstring& message, const std::wstring& caption, uint32_t style);

	static DialogResult show(const std::wstring& message, const std::wstring& caption, uint32_t style);
};

}
