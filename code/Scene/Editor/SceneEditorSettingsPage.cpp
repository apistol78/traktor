#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "I18N/Text.h"
#include "Scene/Editor/SceneEditorSettingsPage.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "World/IWorldRenderer.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneEditorSettingsPage", 0, SceneEditorSettingsPage, editor::ISettingsPage)

bool SceneEditorSettingsPage::create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 4, 4)))
		return false;

	m_checkInvertMouseWheel = new ui::CheckBox();
	m_checkInvertMouseWheel->create(container, i18n::Text(L"SCENE_EDITOR_SETTINGS_INVERT_MOUSE_WHEEL"));
	m_checkInvertMouseWheel->setChecked(settings->getProperty< PropertyBoolean >(L"SceneEditor.InvertMouseWheel"));

	m_checkInvertPanY = new ui::CheckBox();
	m_checkInvertPanY->create(container, i18n::Text(L"SCENE_EDITOR_SETTINGS_INVERT_PAN_Y"));
	m_checkInvertPanY->setChecked(settings->getProperty< PropertyBoolean >(L"SceneEditor.InvertPanY"));

	parent->setText(i18n::Text(L"SCENE_EDITOR_SETTINGS"));
	return true;
}

void SceneEditorSettingsPage::destroy()
{
}

bool SceneEditorSettingsPage::apply(PropertyGroup* settings)
{
	settings->setProperty< PropertyBoolean >(L"SceneEditor.InvertMouseWheel", m_checkInvertMouseWheel->isChecked());
	settings->setProperty< PropertyBoolean >(L"SceneEditor.InvertPanY", m_checkInvertPanY->isChecked());
	return true;
}

	}
}
