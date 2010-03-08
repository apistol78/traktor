#include "Core/Misc/SafeDestroy.h"
#include "Editor/IEditor.h"
#include "Editor/Settings.h"
#include "Sound/ISoundDriver.h"
#include "Sound/SoundSystem.h"
#include "Sound/Editor/SoundEditorPlugin.h"
#include "Ui/MessageBox.h"

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
	editor::Settings* settings = m_editor->getSettings();

	std::wstring soundDriverTypeName = settings->getProperty< editor::PropertyString >(L"Editor.SoundDriver");
	const TypeInfo* soundDriverType = TypeInfo::find(soundDriverTypeName);
	if (!soundDriverType)
	{
		ui::MessageBox::show(parent, std::wstring(L"Unable to instantiate sound driver \"") + soundDriverTypeName + std::wstring(L"\"\nNo such type"), L"Error", ui::MbIconError | ui::MbOk);
		return false;
	}

	Ref< ISoundDriver > soundDriver = dynamic_type_cast< ISoundDriver* >(soundDriverType->createInstance());
	T_ASSERT (soundDriver);

	SoundSystemCreateDesc desc;
	desc.channels = settings->getProperty< editor::PropertyInteger >(L"Editor.SoundVirtualChannels", 8);
	desc.driverDesc.sampleRate = settings->getProperty< editor::PropertyInteger >(L"Editor.SoundSampleRate", 44100);
	desc.driverDesc.bitsPerSample = settings->getProperty< editor::PropertyInteger >(L"Editor.SoundBitsPerSample", 16);
	desc.driverDesc.hwChannels = settings->getProperty< editor::PropertyInteger >(L"Editor.SoundHwChannels", 5 + 1);
	desc.driverDesc.frameSamples = settings->getProperty< editor::PropertyInteger >(L"Editor.SoundFrameSamples", 1024);
	desc.driverDesc.mixerFrames = settings->getProperty< editor::PropertyInteger >(L"Editor.SoundMixerFrames", 3);

	Ref< SoundSystem > soundSystem = new SoundSystem(soundDriver);
	if (!soundSystem->create(desc))
	{
		ui::MessageBox::show(parent, std::wstring(L"Unable to create sound system \"") + soundDriverTypeName + std::wstring(L"\""), L"Error", ui::MbIconError | ui::MbOk);
		return false;
	}

	m_editor->setStoreObject(L"SoundSystem", soundSystem);
	return true;
}

void SoundEditorPlugin::destroy()
{
	Ref< SoundSystem > soundSystem = m_editor->getStoreObject< SoundSystem >(L"SoundSystem");
	if (soundSystem)
	{
		safeDestroy(soundSystem);
		m_editor->setStoreObject(L"SoundSystem", 0);
	}
}

bool SoundEditorPlugin::handleCommand(const ui::Command& command)
{
	return false;
}

void SoundEditorPlugin::handleDatabaseEvent(const Guid& eventId)
{
}

	}
}
