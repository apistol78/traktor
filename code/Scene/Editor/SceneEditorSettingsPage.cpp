#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "I18N/Text.h"
#include "Scene/Editor/SceneEditorSettingsPage.h"
#include "Ui/DropDown.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "World/IWorldRenderer.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const float c_defaultFieldOfView = 80.0f;
const float c_defaultMouseWheelRate = 10.0f;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneEditorSettingsPage", 0, SceneEditorSettingsPage, editor::ISettingsPage)

bool SceneEditorSettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	m_container = new ui::Container();
	if (!m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0, 4)))
		return false;

	Ref< ui::Container > containerSliders = new ui::Container();
	containerSliders->create(m_container, ui::WsNone, new ui::TableLayout(L"*,100%,*", L"*", 0, 4));

	Ref< ui::Static > staticWorldRenderer = new ui::Static();
	staticWorldRenderer->create(containerSliders, i18n::Text(L"SCENE_EDITOR_SETTINGS_WORLD_RENDERER"));

	m_dropWorldRenderer = new ui::DropDown();
	m_dropWorldRenderer->create(containerSliders, L"");

	Ref< ui::Static > staticDummy = new ui::Static();
	staticDummy->create(containerSliders, L"");

	Ref< ui::Static > staticFov = new ui::Static();
	staticFov->create(containerSliders, i18n::Text(L"SCENE_EDITOR_SETTINGS_FOV"));

	m_sliderFov = new ui::Slider();
	m_sliderFov->create(containerSliders);
	m_sliderFov->setRange(10, 180);
	m_sliderFov->setValue(int32_t(settings->getProperty< PropertyFloat >(L"SceneEditor.FieldOfView", c_defaultFieldOfView)));
	m_sliderFov->addEventHandler< ui::ContentChangeEvent >(this, &SceneEditorSettingsPage::eventValueChange);

	m_staticFovValue = new ui::Static();
	m_staticFovValue->create(containerSliders, L"");

	Ref< ui::Static > staticMouseWheelRate = new ui::Static();
	staticMouseWheelRate->create(containerSliders, i18n::Text(L"SCENE_EDITOR_SETTINGS_MOUSE_WHEEL_RATE"));

	m_sliderMouseWheelRate = new ui::Slider();
	m_sliderMouseWheelRate->create(containerSliders);
	m_sliderMouseWheelRate->setRange(1, 100);
	m_sliderMouseWheelRate->setValue(int32_t(settings->getProperty< PropertyFloat >(L"SceneEditor.MouseWheelRate", c_defaultMouseWheelRate)));
	m_sliderMouseWheelRate->addEventHandler< ui::ContentChangeEvent >(this, &SceneEditorSettingsPage::eventValueChange);

	m_staticMouseWheelRateValue = new ui::Static();
	m_staticMouseWheelRateValue->create(containerSliders, L"");

	m_checkInvertMouseWheel = new ui::CheckBox();
	m_checkInvertMouseWheel->create(m_container, i18n::Text(L"SCENE_EDITOR_SETTINGS_INVERT_MOUSE_WHEEL"));
	m_checkInvertMouseWheel->setChecked(settings->getProperty< PropertyBoolean >(L"SceneEditor.InvertMouseWheel"));

	m_checkInvertPanY = new ui::CheckBox();
	m_checkInvertPanY->create(m_container, i18n::Text(L"SCENE_EDITOR_SETTINGS_INVERT_PAN_Y"));
	m_checkInvertPanY->setChecked(settings->getProperty< PropertyBoolean >(L"SceneEditor.InvertPanY"));

	m_checkBuildWhenDrop = new ui::CheckBox();
	m_checkBuildWhenDrop->create(m_container, i18n::Text(L"SCENE_EDITOR_SETTINGS_BUILD_WHEN_DROP"));
	m_checkBuildWhenDrop->setChecked(settings->getProperty< PropertyBoolean >(L"SceneEditor.BuildWhenDrop", true));

	m_checkBuildNavMesh = new ui::CheckBox();
	m_checkBuildNavMesh->create(m_container, i18n::Text(L"SCENE_EDITOR_SETTINGS_BUILD_NAVMESH"));
	m_checkBuildNavMesh->setChecked(settings->getProperty< PropertyBoolean >(L"NavMeshPipeline.Build", true));

	m_checkBuildLighting = new ui::CheckBox();
	m_checkBuildLighting->create(m_container, i18n::Text(L"SCENE_EDITOR_SETTINGS_BUILD_LIGHTING"));
	m_checkBuildLighting->setChecked(settings->getProperty< PropertyBoolean >(L"IlluminatePipeline.Build", true));

	parent->setText(i18n::Text(L"SCENE_EDITOR_SETTINGS"));

	std::wstring worldRendererTypeName = settings->getProperty< PropertyString >(L"SceneEditor.WorldRendererType", L"traktor.world.WorldRendererDeferred");

	TypeInfoSet worldRendererTypes;
	type_of< world::IWorldRenderer >().findAllOf(worldRendererTypes, false);

	for (TypeInfoSet::const_iterator i = worldRendererTypes.begin(); i != worldRendererTypes.end(); ++i)
	{
		std::wstring name = (*i)->getName();
		int32_t index = m_dropWorldRenderer->add(name);
		if (name == worldRendererTypeName)
			m_dropWorldRenderer->select(index);
	}

	updateValues();
	return true;
}

void SceneEditorSettingsPage::destroy()
{
}

bool SceneEditorSettingsPage::apply(PropertyGroup* settings)
{
	settings->setProperty< PropertyString >(L"SceneEditor.WorldRendererType", m_dropWorldRenderer->getSelectedItem());
	settings->setProperty< PropertyFloat >(L"SceneEditor.FieldOfView", float(m_sliderFov->getValue()));
	settings->setProperty< PropertyFloat >(L"SceneEditor.MouseWheelRate", float(m_sliderMouseWheelRate->getValue()));
	settings->setProperty< PropertyBoolean >(L"SceneEditor.InvertMouseWheel", m_checkInvertMouseWheel->isChecked());
	settings->setProperty< PropertyBoolean >(L"SceneEditor.InvertPanY", m_checkInvertPanY->isChecked());
	settings->setProperty< PropertyBoolean >(L"SceneEditor.BuildWhenDrop", m_checkBuildWhenDrop->isChecked());
	settings->setProperty< PropertyBoolean >(L"NavMeshPipeline.Build", m_checkBuildNavMesh->isChecked());
	settings->setProperty< PropertyBoolean >(L"IlluminatePipeline.Build", m_checkBuildLighting->isChecked());
	return true;
}

void SceneEditorSettingsPage::updateValues()
{
	int32_t fov = m_sliderFov->getValue();
	m_staticFovValue->setText(toString(fov) + L" degree(s)");
	m_staticFovValue->update();

	int32_t mouseWheelRate = m_sliderMouseWheelRate->getValue();
	m_staticMouseWheelRateValue->setText(toString(mouseWheelRate));
	m_staticMouseWheelRateValue->update();

	m_container->update();
}

void SceneEditorSettingsPage::eventValueChange(ui::ContentChangeEvent* event)
{
	updateValues();
}

	}
}
