/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Sound/AudioSystem.h"
#include "Sound/IAudioDriver.h"
#include "Sound/Player/SoundPlayer.h"
#include "Sound/Editor/SoundEditorPlugin.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundEditorPlugin", SoundEditorPlugin, editor::IEditorPlugin)

SoundEditorPlugin::SoundEditorPlugin(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool SoundEditorPlugin::create(ui::Widget* parent, editor::IEditorPageSite* site)
{
	return true;
}

void SoundEditorPlugin::destroy()
{
	Ref< AudioSystem > audioSystem = m_editor->getStoreObject< AudioSystem >(L"AudioSystem");
	safeDestroy(audioSystem);
	m_editor->setStoreObject(L"AudioSystem", 0);
}

bool SoundEditorPlugin::handleCommand(const ui::Command& command, bool result)
{
	return false;
}

void SoundEditorPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void SoundEditorPlugin::handleWorkspaceOpened()
{
	Ref< const PropertyGroup > settings = m_editor->getSettings();
	T_ASSERT(settings);

	if (m_editor->getStoreObject(L"AudioSystem") != 0)
		return;

	std::wstring audioDriverTypeName = settings->getProperty< std::wstring >(L"Editor.AudioDriver");
	const TypeInfo* audioDriverType = TypeInfo::find(audioDriverTypeName.c_str());
	if (!audioDriverType)
		return;

	Ref< IAudioDriver > audioDriver = dynamic_type_cast< IAudioDriver* >(audioDriverType->createInstance());
	T_ASSERT(audioDriver);

	SoundSystemCreateDesc desc;
	desc.channels = settings->getProperty< int32_t >(L"Editor.SoundVirtualChannels", 8);
	desc.driverDesc.sampleRate = settings->getProperty< int32_t >(L"Editor.SoundSampleRate", 44100);
	desc.driverDesc.bitsPerSample = settings->getProperty< int32_t >(L"Editor.SoundBitsPerSample", 16);
	desc.driverDesc.hwChannels = settings->getProperty< int32_t >(L"Editor.SoundHwChannels", 5 + 1);
	desc.driverDesc.frameSamples = settings->getProperty< int32_t >(L"Editor.SoundFrameSamples", 1024);

	Ref< AudioSystem > audioSystem = new AudioSystem(audioDriver);
	if (!audioSystem->create(desc))
		return;

	Ref< SoundPlayer > soundPlayer = new SoundPlayer();
	if (!soundPlayer->create(audioSystem, 0))
		return;

	m_editor->setStoreObject(L"AudioSystem", audioSystem);
	m_editor->setStoreObject(L"SoundPlayer", soundPlayer);
}

void SoundEditorPlugin::handleWorkspaceClosed()
{
}

	}
}
