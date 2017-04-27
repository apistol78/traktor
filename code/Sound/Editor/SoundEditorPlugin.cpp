/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Sound/ISoundDriver.h"
#include "Sound/SoundSystem.h"
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
	Ref< SoundSystem > soundSystem = m_editor->getStoreObject< SoundSystem >(L"SoundSystem");
	safeDestroy(soundSystem);
	m_editor->setStoreObject(L"SoundSystem", 0);
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
	T_ASSERT (settings);

	if (m_editor->getStoreObject(L"SoundSystem") != 0)
		return;

	std::wstring soundDriverTypeName = settings->getProperty< std::wstring >(L"Editor.SoundDriver");
	const TypeInfo* soundDriverType = TypeInfo::find(soundDriverTypeName);
	if (!soundDriverType)
		return;

	Ref< ISoundDriver > soundDriver = dynamic_type_cast< ISoundDriver* >(soundDriverType->createInstance());
	T_ASSERT (soundDriver);

	SoundSystemCreateDesc desc;
	desc.channels = settings->getProperty< int32_t >(L"Editor.SoundVirtualChannels", 8);
	desc.driverDesc.sampleRate = settings->getProperty< int32_t >(L"Editor.SoundSampleRate", 44100);
	desc.driverDesc.bitsPerSample = settings->getProperty< int32_t >(L"Editor.SoundBitsPerSample", 16);
	desc.driverDesc.hwChannels = settings->getProperty< int32_t >(L"Editor.SoundHwChannels", 5 + 1);
	desc.driverDesc.frameSamples = settings->getProperty< int32_t >(L"Editor.SoundFrameSamples", 1024);

	Ref< SoundSystem > soundSystem = new SoundSystem(soundDriver);
	if (!soundSystem->create(desc))
		return;

	Ref< SoundPlayer > soundPlayer = new SoundPlayer();
	if (!soundPlayer->create(soundSystem, 0))
		return;

	m_editor->setStoreObject(L"SoundSystem", soundSystem);
	m_editor->setStoreObject(L"SoundPlayer", soundPlayer);
}

void SoundEditorPlugin::handleWorkspaceClosed()
{
}

	}
}
