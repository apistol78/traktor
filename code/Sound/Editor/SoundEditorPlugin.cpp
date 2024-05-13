/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/ThreadManager.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Sound/AudioSystem.h"
#include "Sound/IAudioDriver.h"
#include "Sound/Player/SoundPlayer.h"
#include "Sound/Editor/SoundEditorPlugin.h"
#include "Sound/Editor/SoundPanel.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundEditorPlugin", 0, SoundEditorPlugin, editor::IEditorPlugin)

bool SoundEditorPlugin::create(editor::IEditor* editor, ui::Widget* parent, editor::IEditorPageSite* site)
{
	m_editor = editor;
	m_site = site;

	m_soundPanel = new SoundPanel(m_editor);
	m_soundPanel->create(parent);

	m_site->createAdditionalPanel(m_soundPanel, 60_ut, false);
	return true;
}

void SoundEditorPlugin::destroy()
{
	Ref< AudioSystem > audioSystem = m_editor->getObjectStore()->get< AudioSystem >();

	if (m_soundPanel)
	{
		m_site->destroyAdditionalPanel(m_soundPanel);
		m_site = nullptr;
		m_soundPanel = nullptr;
	}

	if (m_threadPlayer)
	{
		m_threadPlayer->stop();
		ThreadManager::getInstance().destroy(m_threadPlayer);
		m_threadPlayer = nullptr;
	}

	safeDestroy(audioSystem);

	m_editor->getObjectStore()->unset(audioSystem);
	m_editor->getObjectStore()->unset< SoundPlayer >();
}

int32_t SoundEditorPlugin::getOrdinal() const
{
	return 0;
}

void SoundEditorPlugin::getCommands(std::list< ui::Command >& outCommands) const
{
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

	if (m_editor->getObjectStore()->get< AudioSystem >() != nullptr)
		return;

	const std::wstring audioDriverTypeName = settings->getProperty< std::wstring >(L"Editor.AudioDriver");
	const TypeInfo* audioDriverType = TypeInfo::find(audioDriverTypeName.c_str());
	if (!audioDriverType)
		return;

	Ref< IAudioDriver > audioDriver = dynamic_type_cast< IAudioDriver* >(audioDriverType->createInstance());
	T_ASSERT(audioDriver);

	AudioSystemCreateDesc desc;
	desc.channels = settings->getProperty< int32_t >(L"Editor.SoundVirtualChannels", 8);
	desc.driverDesc.sampleRate = settings->getProperty< int32_t >(L"Editor.SoundSampleRate", 44100);
	desc.driverDesc.bitsPerSample = settings->getProperty< int32_t >(L"Editor.SoundBitsPerSample", 16);
	desc.driverDesc.hwChannels = settings->getProperty< int32_t >(L"Editor.SoundHwChannels", 5 + 1);
	desc.driverDesc.frameSamples = settings->getProperty< int32_t >(L"Editor.SoundFrameSamples", 1024);

	Ref< AudioSystem > audioSystem = new AudioSystem(audioDriver);
	if (!audioSystem->create(desc))
		return;

	Ref< SoundPlayer > soundPlayer = new SoundPlayer();
	if (!soundPlayer->create(audioSystem, nullptr))
		return;

	m_editor->getObjectStore()->set(audioSystem);
	m_editor->getObjectStore()->set(soundPlayer);

	m_threadPlayer = ThreadManager::getInstance().create(
		[this, soundPlayer]()
		{
			while (!m_threadPlayer->stopped())
			{
				m_threadPlayer->sleep(30);
				soundPlayer->update(1.0f / 30.0f);
			}
		},
		L"Sound player thread"
	);
	m_threadPlayer->start();
}

void SoundEditorPlugin::handleWorkspaceClosed()
{
}

void SoundEditorPlugin::handleEditorClosed()
{
}

}
