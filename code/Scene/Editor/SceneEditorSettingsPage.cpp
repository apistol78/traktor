#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "I18N/Text.h"
#include "Scene/Editor/SceneEditorSettingsPage.h"
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

bool SceneEditorSettingsPage::create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 4, 4)))
		return false;

	Ref< ui::Container > containerSliders = new ui::Container();
	containerSliders->create(container, ui::WsNone, new ui::TableLayout(L"*,300,*", L"*", 0, 4));

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
	m_checkInvertMouseWheel->create(container, i18n::Text(L"SCENE_EDITOR_SETTINGS_INVERT_MOUSE_WHEEL"));
	m_checkInvertMouseWheel->setChecked(settings->getProperty< PropertyBoolean >(L"SceneEditor.InvertMouseWheel"));

	m_checkInvertPanY = new ui::CheckBox();
	m_checkInvertPanY->create(container, i18n::Text(L"SCENE_EDITOR_SETTINGS_INVERT_PAN_Y"));
	m_checkInvertPanY->setChecked(settings->getProperty< PropertyBoolean >(L"SceneEditor.InvertPanY"));

	m_checkBuildWhenDrop = new ui::CheckBox();
	m_checkBuildWhenDrop->create(container, i18n::Text(L"SCENE_EDITOR_SETTINGS_BUILD_WHEN_DROP"));
	m_checkBuildWhenDrop->setChecked(settings->getProperty< PropertyBoolean >(L"SceneEditor.BuildWhenDrop", true));

	parent->setText(i18n::Text(L"SCENE_EDITOR_SETTINGS"));

	updateValues();
	return true;
}

void SceneEditorSettingsPage::destroy()
{
}

bool SceneEditorSettingsPage::apply(PropertyGroup* settings)
{
	settings->setProperty< PropertyFloat >(L"SceneEditor.FieldOfView", float(m_sliderFov->getValue()));
	settings->setProperty< PropertyFloat >(L"SceneEditor.MouseWheelRate", float(m_sliderMouseWheelRate->getValue()));
	settings->setProperty< PropertyBoolean >(L"SceneEditor.InvertMouseWheel", m_checkInvertMouseWheel->isChecked());
	settings->setProperty< PropertyBoolean >(L"SceneEditor.InvertPanY", m_checkInvertPanY->isChecked());
	settings->setProperty< PropertyBoolean >(L"SceneEditor.BuildWhenDrop", m_checkBuildWhenDrop->isChecked());
	return true;
}

void SceneEditorSettingsPage::updateValues()
{
	int32_t fov = m_sliderFov->getValue();
	m_staticFovValue->setText(toString(fov) + L" degree(s)");

	int32_t mouseWheelRate = m_sliderMouseWheelRate->getValue();
	m_staticMouseWheelRateValue->setText(toString(mouseWheelRate));
}

void SceneEditorSettingsPage::eventValueChange(ui::ContentChangeEvent* event)
{
	updateValues();
}

	}
}
