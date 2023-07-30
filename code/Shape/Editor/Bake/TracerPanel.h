/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{
	
class Button;
class ProgressBar;

	}

	namespace shape
	{

class TracerPanel : public ui::Container
{
	T_RTTI_CLASS;

public:
	explicit TracerPanel(editor::IEditor* editor);

	bool create(ui::Widget* parent);

private:
	editor::IEditor* m_editor;
	Ref< ui::ProgressBar > m_progressBar;
	Ref< ui::Button > m_buttonAbort;
	bool m_idle;
};

	}
}
