#include "Editor/IEditor.h"
#include "Sound/Editor/SoundEditorPlugin.h"
#include "Sound/Editor/SoundSystemFactory.h"

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
	m_editor->setStoreObject(L"SoundSystemFactory", new SoundSystemFactory(m_editor));
	return true;
}

void SoundEditorPlugin::destroy()
{
	m_editor->setStoreObject(L"SoundSystemFactory", 0);
}

bool SoundEditorPlugin::handleCommand(const ui::Command& command, bool result)
{
	return false;
}

void SoundEditorPlugin::handleDatabaseEvent(const Guid& eventId)
{
}

	}
}
