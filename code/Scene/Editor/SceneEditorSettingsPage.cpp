#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "I18N/Text.h"
#include "Scene/Editor/SceneEditorSettingsPage.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/DropDown.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "World/IWorldRenderer.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneEditorSettingsPage", 0, SceneEditorSettingsPage, editor::ISettingsPage)

bool SceneEditorSettingsPage::create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0, 4)))
		return false;

	Ref< ui::Container > container2 = new ui::Container();
	container2->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4));

	Ref< ui::Static > staticWorldRenderer = new ui::Static();
	staticWorldRenderer->create(container2, i18n::Text(L"SCENE_EDITOR_SETTINGS_WORLDRENDERER_TYPE"));

	m_dropWorldRenderer = new ui::DropDown();
	m_dropWorldRenderer->create(container2, L"");

	m_checkInvertMouseWheel = new ui::CheckBox();
	m_checkInvertMouseWheel->create(container, i18n::Text(L"SCENE_EDITOR_SETTINGS_INVERT_MOUSE_WHEEL"));
	m_checkInvertMouseWheel->setChecked(settings->getProperty< PropertyBoolean >(L"SceneEditor.InvertMouseWheel"));

	std::wstring worldRendererType = settings->getProperty< PropertyString >(L"Editor.WorldRenderer");

	std::vector< const TypeInfo* > worldRendererTypes;
	type_of< world::IWorldRenderer >().findAllOf(worldRendererTypes, false);

	for (std::vector< const TypeInfo* >::const_iterator i = worldRendererTypes.begin(); i != worldRendererTypes.end(); ++i)
	{
		std::wstring name = (*i)->getName();
		int32_t index = m_dropWorldRenderer->add(name);
		if (name == worldRendererType)
			m_dropWorldRenderer->select(index);
	}

	parent->setText(i18n::Text(L"SCENE_EDITOR_SETTINGS"));
	return true;
}

void SceneEditorSettingsPage::destroy()
{
}

bool SceneEditorSettingsPage::apply(Settings* settings)
{
	settings->setProperty< PropertyBoolean >(L"SceneEditor.InvertMouseWheel", m_checkInvertMouseWheel->isChecked());
	settings->setProperty< PropertyString >(L"Editor.WorldRenderer", m_dropWorldRenderer->getSelectedItem());
	return true;
}

	}
}
