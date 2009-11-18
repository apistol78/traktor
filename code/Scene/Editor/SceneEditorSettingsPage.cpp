#include "Scene/Editor/SceneEditorSettingsPage.h"
#include "Editor/Settings.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/CheckBox.h"
#include "I18N/Text.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneEditorSettingsPage", 0, SceneEditorSettingsPage, editor::ISettingsPage)

bool SceneEditorSettingsPage::create(ui::Container* parent, editor::Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0, 4)))
		return false;

	m_checkInvertMouseWheel = new ui::CheckBox();
	m_checkInvertMouseWheel->create(container, i18n::Text(L"SCENE_EDITOR_SETTINGS_INVERT_MOUSE_WHEEL"));
	m_checkInvertMouseWheel->setChecked(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.InvertMouseWheel"));

	parent->setText(i18n::Text(L"SCENE_EDITOR_SETTINGS"));
	return true;
}

void SceneEditorSettingsPage::destroy()
{
}

bool SceneEditorSettingsPage::apply(editor::Settings* settings)
{
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.InvertMouseWheel", m_checkInvertMouseWheel->isChecked());
	return true;
}

	}
}
