/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Ui/Dialog.h"

namespace traktor
{
	namespace ui
	{

class LogList;

	}

	namespace amalgam
	{

class ErrorDialog : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	bool create();

	void addErrorString(const std::wstring& errorString);

private:
	Ref< ui::LogList > m_listLog;

	void eventButtonCopyQuit(ui::ButtonClickEvent* event);

	void eventButtonClickQuit(ui::ButtonClickEvent* event);
};

	}
}

