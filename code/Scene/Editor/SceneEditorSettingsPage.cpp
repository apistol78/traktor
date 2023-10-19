/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "I18N/Text.h"
#include "Scene/Editor/SceneEditorSettingsPage.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/DropDown.h"
#include "World/IWorldRenderer.h"

namespace traktor::scene
{
	namespace
	{

const float c_defaultFieldOfView = 80.0f;
const float c_defaultMovementSpeed = 40.0f;
const float c_defaultMouseWheelRate = 10.0f;

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneEditorSettingsPage", 0, SceneEditorSettingsPage, editor::ISettingsPage)

bool SceneEditorSettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	m_container = new ui::Container();
	if (!m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 4_ut, 4_ut)))
		return false;

	Ref< ui::Container > containerSliders = new ui::Container();
	containerSliders->create(m_container, ui::WsNone, new ui::TableLayout(L"*,100%,*", L"*", 0_ut, 4_ut));

	Ref< ui::Static > staticWorldRenderer = new ui::Static();
	staticWorldRenderer->create(containerSliders, i18n::Text(L"SCENE_EDITOR_SETTINGS_WORLD_RENDERER"));

	m_dropWorldRenderer = new ui::DropDown();
	m_dropWorldRenderer->create(containerSliders);

	Ref< ui::Static > staticDummy = new ui::Static();
	staticDummy->create(containerSliders, L"");

	Ref< ui::Static > staticFov = new ui::Static();
	staticFov->create(containerSliders, i18n::Text(L"SCENE_EDITOR_SETTINGS_FOV"));

	m_sliderFov = new ui::Slider();
	m_sliderFov->create(containerSliders);
	m_sliderFov->setRange(10, 180);
	m_sliderFov->setValue(int32_t(settings->getProperty< float >(L"SceneEditor.FieldOfView", c_defaultFieldOfView)));
	m_sliderFov->addEventHandler< ui::ContentChangeEvent >(this, &SceneEditorSettingsPage::eventValueChange);

	m_staticFovValue = new ui::Static();
	m_staticFovValue->create(containerSliders, L"");

	Ref< ui::Static > staticMovementSpeed = new ui::Static();
	staticMovementSpeed->create(containerSliders, i18n::Text(L"SCENE_EDITOR_SETTINGS_MOVEMENT_SPEED"));

	m_sliderMovementSpeed = new ui::Slider();
	m_sliderMovementSpeed->create(containerSliders);
	m_sliderMovementSpeed->setRange(1, 1000);
	m_sliderMovementSpeed->setValue(int32_t(settings->getProperty< float >(L"SceneEditor.MovementSpeed", c_defaultMovementSpeed)));
	m_sliderMovementSpeed->addEventHandler< ui::ContentChangeEvent >(this, &SceneEditorSettingsPage::eventValueChange);

	m_staticMovementSpeedValue = new ui::Static();
	m_staticMovementSpeedValue->create(containerSliders, L"");

	Ref< ui::Static > staticMouseWheelRate = new ui::Static();
	staticMouseWheelRate->create(containerSliders, i18n::Text(L"SCENE_EDITOR_SETTINGS_MOUSE_WHEEL_RATE"));

	m_sliderMouseWheelRate = new ui::Slider();
	m_sliderMouseWheelRate->create(containerSliders);
	m_sliderMouseWheelRate->setRange(1, 100);
	m_sliderMouseWheelRate->setValue(int32_t(settings->getProperty< float >(L"SceneEditor.MouseWheelRate", c_defaultMouseWheelRate)));
	m_sliderMouseWheelRate->addEventHandler< ui::ContentChangeEvent >(this, &SceneEditorSettingsPage::eventValueChange);

	m_staticMouseWheelRateValue = new ui::Static();
	m_staticMouseWheelRateValue->create(containerSliders, L"");

	m_checkInvertMouseWheel = new ui::CheckBox();
	m_checkInvertMouseWheel->create(m_container, i18n::Text(L"SCENE_EDITOR_SETTINGS_INVERT_MOUSE_WHEEL"));
	m_checkInvertMouseWheel->setChecked(settings->getProperty< bool >(L"SceneEditor.InvertMouseWheel"));

	m_checkInvertPanY = new ui::CheckBox();
	m_checkInvertPanY->create(m_container, i18n::Text(L"SCENE_EDITOR_SETTINGS_INVERT_PAN_Y"));
	m_checkInvertPanY->setChecked(settings->getProperty< bool >(L"SceneEditor.InvertPanY"));

	m_checkBuildWhenDrop = new ui::CheckBox();
	m_checkBuildWhenDrop->create(m_container, i18n::Text(L"SCENE_EDITOR_SETTINGS_BUILD_WHEN_DROP"));
	m_checkBuildWhenDrop->setChecked(settings->getProperty< bool >(L"SceneEditor.BuildWhenDrop", true));

	m_checkBuildNavMesh = new ui::CheckBox();
	m_checkBuildNavMesh->create(m_container, i18n::Text(L"SCENE_EDITOR_SETTINGS_BUILD_NAVMESH"));
	m_checkBuildNavMesh->setChecked(settings->getProperty< bool >(L"NavMeshPipeline.Build", true));

	Ref< ui::Container > containerQuality = new ui::Container();
	containerQuality->create(m_container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0_ut, 4_ut));

	Ref< ui::Static > staticPostProcess = new ui::Static();
	staticPostProcess->create(containerQuality, i18n::Text(L"SCENE_EDITOR_POST_PROCESS"));

	m_dropDownPostProcess = new ui::DropDown();
	m_dropDownPostProcess->create(containerQuality);
	m_dropDownPostProcess->add(L"Disabled");
	m_dropDownPostProcess->add(L"Low");
	m_dropDownPostProcess->add(L"Medium");
	m_dropDownPostProcess->add(L"High");
	m_dropDownPostProcess->add(L"Ultra");
	m_dropDownPostProcess->select(settings->getProperty< int32_t >(L"SceneEditor.PostProcessQuality", 4));

	Ref< ui::Static > staticMotionBlur = new ui::Static();
	staticMotionBlur->create(containerQuality, i18n::Text(L"SCENE_EDITOR_MOTION_BLUR"));

	m_dropDownMotionBlur = new ui::DropDown();
	m_dropDownMotionBlur->create(containerQuality);
	m_dropDownMotionBlur->add(L"Disabled");
	m_dropDownMotionBlur->add(L"Low");
	m_dropDownMotionBlur->add(L"Medium");
	m_dropDownMotionBlur->add(L"High");
	m_dropDownMotionBlur->add(L"Ultra");
	m_dropDownMotionBlur->select(settings->getProperty< int32_t >(L"SceneEditor.MotionBlurQuality", 4));

	Ref< ui::Static > staticShadows = new ui::Static();
	staticShadows->create(containerQuality, i18n::Text(L"SCENE_EDITOR_SHADOWS"));

	m_dropDownShadows = new ui::DropDown();
	m_dropDownShadows->create(containerQuality);
	m_dropDownShadows->add(L"Disabled");
	m_dropDownShadows->add(L"Low");
	m_dropDownShadows->add(L"Medium");
	m_dropDownShadows->add(L"High");
	m_dropDownShadows->add(L"Ultra");
	m_dropDownShadows->select(settings->getProperty< int32_t >(L"SceneEditor.ShadowQuality", 4));

	Ref< ui::Static > staticReflections = new ui::Static();
	staticReflections->create(containerQuality, i18n::Text(L"SCENE_EDITOR_REFLECTIONS"));

	m_dropDownReflections = new ui::DropDown();
	m_dropDownReflections->create(containerQuality);
	m_dropDownReflections->add(L"Disabled");
	m_dropDownReflections->add(L"Low");
	m_dropDownReflections->add(L"Medium");
	m_dropDownReflections->add(L"High");
	m_dropDownReflections->add(L"Ultra");
	m_dropDownReflections->select(settings->getProperty< int32_t >(L"SceneEditor.ReflectionsQuality", 4));

	Ref< ui::Static > staticAmbientOcclusion = new ui::Static();
	staticAmbientOcclusion->create(containerQuality, i18n::Text(L"SCENE_EDITOR_AO"));

	m_dropDownAmbientOcclusion = new ui::DropDown();
	m_dropDownAmbientOcclusion->create(containerQuality);
	m_dropDownAmbientOcclusion->add(L"Disabled");
	m_dropDownAmbientOcclusion->add(L"Low");
	m_dropDownAmbientOcclusion->add(L"Medium");
	m_dropDownAmbientOcclusion->add(L"High");
	m_dropDownAmbientOcclusion->add(L"Ultra");
	m_dropDownAmbientOcclusion->select(settings->getProperty< int32_t >(L"SceneEditor.AmbientOcclusionQuality", 4));

	Ref< ui::Static > staticAntialias = new ui::Static();
	staticAntialias->create(containerQuality, i18n::Text(L"SCENE_EDITOR_AA"));

	m_dropAntialias = new ui::DropDown();
	m_dropAntialias->create(containerQuality);
	m_dropAntialias->add(L"Disabled");
	m_dropAntialias->add(L"Low");
	m_dropAntialias->add(L"Medium");
	m_dropAntialias->add(L"High");
	m_dropAntialias->add(L"Ultra");
	m_dropAntialias->select(settings->getProperty< int32_t >(L"SceneEditor.AntiAliasQuality", 4));

	parent->setText(i18n::Text(L"SCENE_EDITOR_SETTINGS"));

	// Add available world renderer types.
	const std::wstring worldRendererTypeName = settings->getProperty< std::wstring >(L"SceneEditor.WorldRendererType", L"traktor.world.WorldRendererDeferred");
	for (auto worldRendererType : type_of< world::IWorldRenderer >().findAllOf(false))
	{
		if (!worldRendererType->isInstantiable())
			continue;

		const std::wstring name = worldRendererType->getName();
		const int32_t index = m_dropWorldRenderer->add(name);

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
	settings->setProperty< PropertyFloat >(L"SceneEditor.MovementSpeed", float(m_sliderMovementSpeed->getValue()));
	settings->setProperty< PropertyFloat >(L"SceneEditor.MouseWheelRate", float(m_sliderMouseWheelRate->getValue()));
	settings->setProperty< PropertyBoolean >(L"SceneEditor.InvertMouseWheel", m_checkInvertMouseWheel->isChecked());
	settings->setProperty< PropertyBoolean >(L"SceneEditor.InvertPanY", m_checkInvertPanY->isChecked());
	settings->setProperty< PropertyBoolean >(L"SceneEditor.BuildWhenDrop", m_checkBuildWhenDrop->isChecked());
	settings->setProperty< PropertyBoolean >(L"NavMeshPipeline.Build", m_checkBuildNavMesh->isChecked());
	settings->setProperty< PropertyInteger >(L"SceneEditor.PostProcessQuality", m_dropDownPostProcess->getSelected());
	settings->setProperty< PropertyInteger >(L"SceneEditor.MotionBlurQuality", m_dropDownMotionBlur->getSelected());
	settings->setProperty< PropertyInteger >(L"SceneEditor.ShadowQuality", m_dropDownShadows->getSelected());
	settings->setProperty< PropertyInteger >(L"SceneEditor.ReflectionsQuality", m_dropDownReflections->getSelected());
	settings->setProperty< PropertyInteger >(L"SceneEditor.AmbientOcclusionQuality", m_dropDownAmbientOcclusion->getSelected());
	settings->setProperty< PropertyInteger >(L"SceneEditor.AntiAliasQuality", m_dropAntialias->getSelected());
	return true;
}

void SceneEditorSettingsPage::updateValues()
{
	const int32_t fov = m_sliderFov->getValue();
	m_staticFovValue->setText(toString(fov) + L" degree(s)");
	m_staticFovValue->update();

	const int32_t movementSpeed = m_sliderMovementSpeed->getValue();
	m_staticMovementSpeedValue->setText(toString(movementSpeed));
	m_staticMovementSpeedValue->update();

	const int32_t mouseWheelRate = m_sliderMouseWheelRate->getValue();
	m_staticMouseWheelRateValue->setText(toString(mouseWheelRate));
	m_staticMouseWheelRateValue->update();

	m_container->update();
}

void SceneEditorSettingsPage::eventValueChange(ui::ContentChangeEvent* event)
{
	updateValues();
}

}
