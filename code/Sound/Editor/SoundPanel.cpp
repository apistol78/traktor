/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Sound/AudioSystem.h"
#include "Sound/Editor/SoundPanel.h"
#include "Ui/Application.h"
#include "Ui/Slider.h"
#include "Ui/TableLayout.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundPanel", SoundPanel, ui::Container)

SoundPanel::SoundPanel(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool SoundPanel::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%,*", L"100%", 8_ut, 8_ut)))
		return false;

	setText(i18n::Text(L"SOUND_EDITOR_SOUND_PANEL"));

	m_sliderVolume = new ui::Slider();
	m_sliderVolume->create(this);
	m_sliderVolume->setRange(0, 100);
	m_sliderVolume->setValue(100);
	m_sliderVolume->addEventHandler< ui::ContentChangeEvent >([&](ui::ContentChangeEvent* event) {
		AudioSystem* audioSystem = m_editor->getStoreObject< AudioSystem >(L"AudioSystem");
		if (audioSystem)
			audioSystem->setVolume(m_sliderVolume->getValue() / 100.0f);
	});

	update();
	return true;
}

}
