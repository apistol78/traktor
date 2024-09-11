/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/ISettingsPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class DropDown;
class Edit;

}

namespace traktor::sound
{

/*! Editor sound settings page.
 * \ingroup Sound
 */
class T_DLLCLASS SoundSettingsPage : public editor::ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) override final;

	virtual void destroy() override final;

	virtual bool apply(PropertyGroup* settings) override final;

private:
	Ref< ui::DropDown > m_dropAudioDriver;
	Ref< ui::Edit > m_editVirtualChannels;
	Ref< ui::DropDown > m_dropSampleRate;
	Ref< ui::DropDown > m_dropBitsPerSample;
	Ref< ui::DropDown > m_dropHwChannels;
	Ref< ui::Edit > m_editFrameSamples;
	Ref< ui::Edit > m_editMixerFrames;
};

}
