/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Container.h"

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{
	
class Slider;

}

namespace traktor::sound
{

class SoundPanel : public ui::Container
{
	T_RTTI_CLASS;

public:
	explicit SoundPanel(editor::IEditor* editor);

	bool create(ui::Widget* parent);

private:
	editor::IEditor* m_editor;
	Ref< ui::Slider > m_sliderVolume;
};

}
