/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/ISettingsPage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class CheckBox;
class ContentChangeEvent;
class DropDown;
class Slider;
class Static;

}

namespace traktor::scene
{

/*! Scene editor settings page.
 * \ingroup Scene
 */
class SceneEditorSettingsPage : public editor::ISettingsPage
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands) override final;

	virtual void destroy() override final;

	virtual bool apply(PropertyGroup* settings) override final;

private:
	Ref< ui::Container > m_container;
	Ref< ui::DropDown > m_dropWorldRenderer;
	Ref< ui::Slider > m_sliderFov;
	Ref< ui::Slider > m_sliderMovementSpeed;
	Ref< ui::Static > m_staticMovementSpeedValue;
	Ref< ui::Slider > m_sliderMouseWheelRate;
	Ref< ui::Static > m_staticFovValue;
	Ref< ui::Static > m_staticMouseWheelRateValue;
	Ref< ui::CheckBox > m_checkInvertMouseWheel;
	Ref< ui::CheckBox > m_checkInvertPanY;
	Ref< ui::CheckBox > m_checkBuildWhenDrop;
	Ref< ui::CheckBox > m_checkBuildNavMesh;

	Ref< ui::DropDown > m_dropDownPostProcess;
	Ref< ui::DropDown > m_dropDownMotionBlur;
	Ref< ui::DropDown > m_dropDownShadows;
	Ref< ui::DropDown > m_dropDownReflections;
	Ref< ui::DropDown > m_dropDownAmbientOcclusion;
	Ref< ui::DropDown > m_dropAntialias;

	void updateValues();

	void eventValueChange(ui::ContentChangeEvent* event);
};

}
