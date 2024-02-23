/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/RenderControls/CameraRenderControl.h"
#include "Scene/Editor/RenderControls/DefaultRenderControl.h"
#include "Scene/Editor/RenderControls/FinalRenderControl.h"
#include "Scene/Editor/RenderControls/OrthogonalRenderControl.h"
#include "Scene/Editor/RenderControls/PerspectiveRenderControl.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/Slider.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarDropMenu.h"
#include "Ui/ToolBar/ToolBarEmbed.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "World/Editor/IDebugOverlay.h"

namespace traktor::scene
{
	namespace
	{

const wchar_t* c_worldRendererTypes[] = { L"traktor.world.WorldRendererSimple", L"traktor.world.WorldRendererForward", L"traktor.world.WorldRendererDeferred" };

const float c_aspects[] =
{
	0.0f,
	1.0f,
	4.0f / 3.0f,
	6.0f / 4.0f,
	16.0f / 9.0f,
	16.0f / 10.0f,
	3.0f / 4.0f,
	4.0f / 6.0f,
	9.0f / 16.0f
};

ui::MenuItem* getChecked(ui::MenuItem* menu)
{
	for (int i = 0; i < menu->count(); ++i)
	{
		if (menu->get(i)->isChecked())
			return menu->get(i);
	}
	return nullptr;
}

std::wstring getOverlayText(const TypeInfo* overlayType)
{
	std::wstring id = L"SCENE_EDITOR_OVERLAY_" + replaceAll(toUpper(std::wstring(overlayType->getName())), L".", L"_");
	return i18n::Text(id, overlayType->getName());
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultRenderControl", DefaultRenderControl, ISceneRenderControl)

DefaultRenderControl::DefaultRenderControl()
:	m_cameraId(0)
,	m_viewId(0)
{
}

bool DefaultRenderControl::create(ui::Widget* parent, SceneEditorContext* context, int32_t cameraId, int32_t viewId)
{
	m_context = context;
	T_ASSERT(m_context);

	m_cameraId = cameraId;
	m_viewId = viewId;

	Ref< const PropertyGroup > settings = context->getEditor()->getSettings();
	T_ASSERT(settings);

	// Get initial view type, if it's final then we revert to
	// perspective since final need to load it's own resources.
	int32_t viewType = settings->getProperty< int32_t >(L"SceneEditor.View" + toString(m_viewId), 0);
	if (viewType == 8)
		viewType = 0;

	bool gridEnable = settings->getProperty< bool >(L"Scene.Editor.GridEnable" + toString(m_viewId), true);
	bool guideEnable = settings->getProperty< bool >(L"Scene.Editor.GuideEnable" + toString(m_viewId), true);

	m_container = new ui::Container();
	if (!m_container->create(parent, ui::WsNoCanvas, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	m_container->addEventHandler< ui::SizeEvent >(this, &DefaultRenderControl::eventResize);

	m_toolBar = new ui::ToolBar();
	if (!m_toolBar->create(m_container))
		return false;

	m_toolBar->addImage(new ui::StyleBitmap(L"Scene.ToggleGrid"));
	m_toolBar->addImage(new ui::StyleBitmap(L"Scene.ToggleGuide"));

	m_toolView = new ui::ToolBarDropDown(ui::Command(L"Scene.Editor.View"), 100_ut, i18n::Text(L"SCENE_EDITOR_VIEW_MODE"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_PERSPECTIVE"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_FRONT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_BACK"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_TOP"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_BOTTOM"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_LEFT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_RIGHT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_CAMERA"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_FINAL"));
	m_toolView->select(viewType);

	m_toolToggleGrid = new ui::ToolBarButton(
		i18n::Text(L"SCENE_EDITOR_TOGGLE_GRID"),
		0,
		ui::Command(1, L"Scene.Editor.ToggleGrid"),
		gridEnable ? ui::ToolBarButton::BsDefaultToggled : ui::ToolBarButton::BsDefaultToggle
	);

	m_toolToggleGuide = new ui::ToolBarButton(
		i18n::Text(L"SCENE_EDITOR_TOGGLE_GUIDE"),
		1,
		ui::Command(1, L"Scene.Editor.ToggleGuide"),
		guideEnable ? ui::ToolBarButton::BsDefaultToggled : ui::ToolBarButton::BsDefaultToggle
	);

	m_toolAspect = new ui::ToolBarDropDown(ui::Command(1, L"Scene.Editor.Aspect"), 70_ut, i18n::Text(L"SCENE_EDITOR_ASPECT"));
	m_toolAspect->add(L"Full");
	m_toolAspect->add(L"1:1");
	m_toolAspect->add(L"4:3");
	m_toolAspect->add(L"6:4");
	m_toolAspect->add(L"16:9");
	m_toolAspect->add(L"16:10");
	m_toolAspect->add(L"3:4");
	m_toolAspect->add(L"4:6");
	m_toolAspect->add(L"9:16");
	m_toolAspect->select(0);

	m_toolQualityMenu = new ui::ToolBarDropMenu(130_ut, i18n::Text(L"SCENE_EDITOR_QUALITY"), true, i18n::Text(L"SCENE_EDITOR_QUALITY_TOOLTIP"));

	m_menuPostProcess = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_POST_PROCESS"));
	m_menuPostProcess->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.PostProcessQuality"), L"Disabled", true, 0));
	m_menuPostProcess->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.PostProcessQuality"), L"Low", true, 0));
	m_menuPostProcess->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.PostProcessQuality"), L"Medium", true, 0));
	m_menuPostProcess->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.PostProcessQuality"), L"High", true, 0));
	m_menuPostProcess->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.PostProcessQuality"), L"Ultra", true, 0));
	m_menuPostProcess->get(settings->getProperty< int32_t >(L"SceneEditor.PostProcessQuality", 4))->setChecked(true);
	m_toolQualityMenu->add(m_menuPostProcess);

	m_menuMotionBlur = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_MOTION_BLUR"));
	m_menuMotionBlur->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.MotionBlurQuality"), L"Disabled", true, 0));
	m_menuMotionBlur->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.MotionBlurQuality"), L"Low", true, 0));
	m_menuMotionBlur->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.MotionBlurQuality"), L"Medium", true, 0));
	m_menuMotionBlur->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.MotionBlurQuality"), L"High", true, 0));
	m_menuMotionBlur->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.MotionBlurQuality"), L"Ultra", true, 0));
	m_menuMotionBlur->get(settings->getProperty< int32_t >(L"SceneEditor.MotionBlurQuality", 4))->setChecked(true);
	m_toolQualityMenu->add(m_menuMotionBlur);

	m_menuShadows = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_SHADOWS"));
	m_menuShadows->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.ShadowQuality"), L"Disabled", true, 0));
	m_menuShadows->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.ShadowQuality"), L"Low", true, 0));
	m_menuShadows->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.ShadowQuality"), L"Medium", true, 0));
	m_menuShadows->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.ShadowQuality"), L"High", true, 0));
	m_menuShadows->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.ShadowQuality"), L"Ultra", true, 0));
	m_menuShadows->get(settings->getProperty< int32_t >(L"SceneEditor.ShadowQuality", 4))->setChecked(true);
	m_toolQualityMenu->add(m_menuShadows);

	m_menuReflections = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_REFLECTIONS"));
	m_menuReflections->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.ReflectionsQuality"), L"Disabled", true, 0));
	m_menuReflections->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.ReflectionsQuality"), L"Low", true, 0));
	m_menuReflections->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.ReflectionsQuality"), L"Medium", true, 0));
	m_menuReflections->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.ReflectionsQuality"), L"High", true, 0));
	m_menuReflections->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.ReflectionsQuality"), L"Ultra", true, 0));
	m_menuReflections->get(settings->getProperty< int32_t >(L"SceneEditor.ReflectionsQuality", 4))->setChecked(true);
	m_toolQualityMenu->add(m_menuReflections);

	m_menuAO = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_AO"));
	m_menuAO->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.AmbientOcclusionQuality"), L"Disabled", true, 0));
	m_menuAO->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.AmbientOcclusionQuality"), L"Low", true, 0));
	m_menuAO->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.AmbientOcclusionQuality"), L"Medium", true, 0));
	m_menuAO->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.AmbientOcclusionQuality"), L"High", true, 0));
	m_menuAO->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.AmbientOcclusionQuality"), L"Ultra", true, 0));
	m_menuAO->get(settings->getProperty< int32_t >(L"SceneEditor.AmbientOcclusionQuality", 4))->setChecked(true);
	m_toolQualityMenu->add(m_menuAO);

	m_menuAA = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_AA"));
	m_menuAA->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.AntiAliasQuality"), L"Disabled", true, 0));
	m_menuAA->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.AntiAliasQuality"), L"Low", true, 0));
	m_menuAA->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.AntiAliasQuality"), L"Medium", true, 0));
	m_menuAA->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.AntiAliasQuality"), L"High", true, 0));
	m_menuAA->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.AntiAliasQuality"), L"Ultra", true, 0));
	m_menuAA->get(settings->getProperty< int32_t >(L"SceneEditor.AntiAliasQuality", 4))->setChecked(true);
	m_toolQualityMenu->add(m_menuAA);

	m_toolWorldRenderer = new ui::ToolBarDropDown(ui::Command(1, L"Scene.Editor.WorldRenderer"), 70_ut, i18n::Text(L"SCENE_EDITOR_WORLD_RENDERER"));
	m_toolWorldRenderer->add(L"Simple");
	m_toolWorldRenderer->add(L"Forward");
	m_toolWorldRenderer->add(L"Deferred");

	const std::wstring worldRendererTypeName = settings->getProperty< std::wstring >(L"SceneEditor.WorldRendererType", L"traktor.world.WorldRendererForward");
	for (int32_t i = 0; i < 3; ++i)
	{
		if (worldRendererTypeName == c_worldRendererTypes[i])
			m_toolWorldRenderer->select(i);
	}

	m_toolDebugOverlay = new ui::ToolBarDropDown(ui::Command(1, L"Scene.Editor.DebugOverlay"), 140_ut, i18n::Text(L"SCENE_EDITOR_DEBUG_OVERLAY"));
	m_toolDebugOverlay->add(L"None");
	
	for (const auto overlayType : type_of< world::IDebugOverlay >().findAllOf(false))
		m_toolDebugOverlay->add(getOverlayText(overlayType));

	m_toolDebugOverlay->select(0);

	m_sliderDebugAlpha = new ui::Slider();
	m_sliderDebugAlpha->create(m_toolBar);
	m_sliderDebugAlpha->setRange(0, 100);
	m_sliderDebugAlpha->setValue(100);
	m_sliderDebugAlpha->addEventHandler< ui::ContentChangeEvent >(this, &DefaultRenderControl::eventSliderDebugChange);

	m_sliderDebugMip= new ui::Slider();
	m_sliderDebugMip->create(m_toolBar);
	m_sliderDebugMip->setRange(0, 100);
	m_sliderDebugMip->setValue(100);
	m_sliderDebugMip->addEventHandler< ui::ContentChangeEvent >(this, &DefaultRenderControl::eventSliderDebugChange);

	m_toolBar->addItem(m_toolView);
	m_toolBar->addItem(m_toolToggleGrid);
	m_toolBar->addItem(m_toolToggleGuide);
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(m_toolAspect);
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(m_toolQualityMenu);
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(m_toolWorldRenderer);
	m_toolBar->addItem(new ui::ToolBarSeparator());
	m_toolBar->addItem(m_toolDebugOverlay);
	m_toolBar->addItem(new ui::ToolBarEmbed(m_sliderDebugAlpha, 30_ut));
	m_toolBar->addItem(new ui::ToolBarEmbed(m_sliderDebugMip, 30_ut));

	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &DefaultRenderControl::eventToolClick);

	if (!createRenderControl(viewType))
		return false;

	return true;
}

void DefaultRenderControl::destroy()
{
	Ref< PropertyGroup > settings = m_context->getEditor()->checkoutGlobalSettings();
	T_ASSERT(settings);

	settings->setProperty< PropertyBoolean >(L"Scene.Editor.GridEnable" + toString(m_viewId), m_toolToggleGrid->isToggled());
	settings->setProperty< PropertyBoolean >(L"Scene.Editor.GuideEnable" + toString(m_viewId), m_toolToggleGuide->isToggled());

	m_context->getEditor()->commitGlobalSettings();
	m_toolView = nullptr;

	safeDestroy(m_renderControl);
	safeDestroy(m_container);
}

void DefaultRenderControl::setWorldRendererType(const TypeInfo& worldRendererType)
{
	if (m_renderControl)
		m_renderControl->setWorldRendererType(worldRendererType);
}

void DefaultRenderControl::setAspect(float aspect)
{
	if (m_renderControl)
		m_renderControl->setAspect(aspect);
}

void DefaultRenderControl::setQuality(world::Quality imageProcess, world::Quality shadows, world::Quality reflections, world::Quality motionBlur, world::Quality ambientOcclusion, world::Quality antiAlias)
{
	if (m_renderControl)
		m_renderControl->setQuality(
			imageProcess,
			shadows,
			reflections,
			motionBlur,
			ambientOcclusion,
			antiAlias
		);
}

void DefaultRenderControl::setDebugOverlay(world::IDebugOverlay* overlay)
{
	if (m_renderControl)
		m_renderControl->setDebugOverlay(overlay);
}

void DefaultRenderControl::setDebugOverlayAlpha(float alpha, float mip)
{
	m_sliderDebugAlpha->setValue((int32_t)(alpha * 100.0f));
	m_sliderDebugMip->setValue((int32_t)(mip * 100.0f));
}

bool DefaultRenderControl::handleCommand(const ui::Command& command)
{
	if (m_renderControl)
		return m_renderControl->handleCommand(command);
	else
		return false;
}

void DefaultRenderControl::update()
{
	const float alpha = m_sliderDebugAlpha->getValue() / 100.0f;
	const float mip = m_sliderDebugMip->getValue() / 100.0f;
	if (m_renderControl)
	{
		m_renderControl->setDebugOverlayAlpha(alpha, mip);
		m_renderControl->update();
	}
}

bool DefaultRenderControl::hitTest(const ui::Point& position) const
{
	return m_renderControl ? m_renderControl->hitTest(position) : false;
}

bool DefaultRenderControl::calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const
{
	return m_renderControl ? m_renderControl->calculateRay(position, outWorldRayOrigin, outWorldRayDirection) : false;
}

bool DefaultRenderControl::calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const
{
	return m_renderControl ? m_renderControl->calculateFrustum(rc, outWorldFrustum) : false;
}

void DefaultRenderControl::moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta)
{
	if (m_renderControl)
		m_renderControl->moveCamera(mode, mouseDelta, viewDelta);
}

void DefaultRenderControl::showSelectionRectangle(const ui::Rect& rect)
{
	if (m_renderControl)
		m_renderControl->showSelectionRectangle(rect);
}

bool DefaultRenderControl::createRenderControl(int32_t type)
{
	safeDestroy(m_renderControl);

	const TypeInfo* worldRendererType = TypeInfo::find(c_worldRendererTypes[m_toolWorldRenderer->getSelected()]);
	if (!worldRendererType)
		return false;

	// Enable all tools by default.
	m_toolToggleGrid->setEnable(true);
	m_toolToggleGuide->setEnable(true);
	m_toolView->setEnable(true);
	m_toolAspect->setEnable(true);
	m_toolQualityMenu->setEnable(true);
	m_toolWorldRenderer->setEnable(true);

	bool toolsEnable = true;

	switch (type)
	{
	default:
	case 0: // Perspective
		{
			Ref< PerspectiveRenderControl > renderControl = new PerspectiveRenderControl();
			if (!renderControl->create(m_container, m_context, m_cameraId, *worldRendererType))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 1:	// Front
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveZ, m_cameraId, *worldRendererType))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 2:	// Back
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeZ, m_cameraId, *worldRendererType))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 3:	// Top
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveY, m_cameraId, *worldRendererType))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 4:	// Bottom
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeY, m_cameraId, *worldRendererType))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 5:	// Left
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveX, m_cameraId, *worldRendererType))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 6:	// Right
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeX, m_cameraId, *worldRendererType))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 7:	// Camera
		{
			Ref< CameraRenderControl > renderControl = new CameraRenderControl();
			if (!renderControl->create(m_container, m_context, *worldRendererType))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 8: // Final
		{
			Ref< FinalRenderControl > renderControl = new FinalRenderControl();
			if (!renderControl->create(m_container, m_context, m_cameraId, *worldRendererType))
				return false;
			m_renderControl = renderControl;

			// Some tools not applicable to final view.
			m_toolToggleGrid->setEnable(false);
			m_toolToggleGuide->setEnable(false);
		}
		break;
	}

	T_ASSERT(m_renderControl);

	m_renderControl->setAspect(c_aspects[m_toolAspect->getSelected()]);

	m_container->update();

	if (m_toolToggleGrid->isToggled())
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnableGrid"));
	else
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisableGrid"));

	if (m_toolToggleGuide->isToggled())
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnableGuide"));
	else
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisableGuide"));

	m_renderControl->setQuality(
		(world::Quality)getChecked(m_menuPostProcess)->getCommand().getId(),
		(world::Quality)getChecked(m_menuShadows)->getCommand().getId(),
		(world::Quality)getChecked(m_menuReflections)->getCommand().getId(),
		(world::Quality)getChecked(m_menuMotionBlur)->getCommand().getId(),
		(world::Quality)getChecked(m_menuAO)->getCommand().getId(),
		(world::Quality)getChecked(m_menuAA)->getCommand().getId()
	);

	{
		Ref< PropertyGroup > settings = m_context->getEditor()->checkoutGlobalSettings();
		T_ASSERT(settings);

		settings->setProperty< PropertyInteger >(L"SceneEditor.View" + toString(m_viewId), type);

		m_context->getEditor()->commitGlobalSettings();
	}

	return true;
}

void DefaultRenderControl::eventResize(ui::SizeEvent* event)
{
	m_context->enqueueRedraw(nullptr);
}

void DefaultRenderControl::eventSliderDebugChange(ui::ContentChangeEvent* event)
{
	m_context->enqueueRedraw(nullptr);
}

void DefaultRenderControl::eventToolClick(ui::ToolBarButtonClickEvent* event)
{
	bool updateQuality = false;

	if (event->getCommand() == L"Scene.Editor.View")
	{
		int32_t selected = m_toolView->getSelected();
		T_ASSERT(selected >= 0);
		createRenderControl(selected);
	}
	else if (event->getCommand() == L"Scene.Editor.ToggleGrid")
	{
		if (m_toolToggleGrid->isToggled())
			m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnableGrid"));
		else
			m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisableGrid"));
	}
	else if (event->getCommand() == L"Scene.Editor.ToggleGuide")
	{
		if (m_toolToggleGuide->isToggled())
			m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnableGuide"));
		else
			m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisableGuide"));
	}
	else if (event->getCommand() == L"Scene.Editor.Aspect")
	{
		m_renderControl->setAspect(c_aspects[m_toolAspect->getSelected()]);
	}
	else if (event->getCommand() == L"Scene.Editor.PostProcessQuality")
	{
		for (int i = 0; i < m_menuPostProcess->count(); ++i)
			m_menuPostProcess->get(i)->setChecked(bool(i == event->getCommand().getId()));
		updateQuality = true;
	}
	else if (event->getCommand() == L"Scene.Editor.MotionBlurQuality")
	{
		for (int i = 0; i < m_menuMotionBlur->count(); ++i)
			m_menuMotionBlur->get(i)->setChecked(bool(i == event->getCommand().getId()));
		updateQuality = true;
	}
	else if (event->getCommand() == L"Scene.Editor.ShadowQuality")
	{
		for (int i = 0; i < m_menuShadows->count(); ++i)
			m_menuShadows->get(i)->setChecked(bool(i == event->getCommand().getId()));
		updateQuality = true;
	}
	else if (event->getCommand() == L"Scene.Editor.ReflectionsQuality")
	{
		for (int i = 0; i < m_menuReflections->count(); ++i)
			m_menuReflections->get(i)->setChecked(bool(i == event->getCommand().getId()));
		updateQuality = true;
	}
	else if (event->getCommand() == L"Scene.Editor.AmbientOcclusionQuality")
	{
		for (int i = 0; i < m_menuAO->count(); ++i)
			m_menuAO->get(i)->setChecked(bool(i == event->getCommand().getId()));
		updateQuality = true;
	}
	else if (event->getCommand() == L"Scene.Editor.AntiAliasQuality")
	{
		for (int i = 0; i < m_menuAA->count(); ++i)
			m_menuAA->get(i)->setChecked(bool(i == event->getCommand().getId()));
		updateQuality = true;
	}
	else if (event->getCommand() == L"Scene.Editor.WorldRenderer")
	{
		const TypeInfo* worldRendererType = TypeInfo::find(c_worldRendererTypes[m_toolWorldRenderer->getSelected()]);
		if (worldRendererType)
			m_renderControl->setWorldRendererType(*worldRendererType);
	}
	else if (event->getCommand() == L"Scene.Editor.DebugOverlay")
	{
		int32_t index = m_toolDebugOverlay->getSelected();
		if (index > 0)
		{
			TypeInfoSet overlayTypes = type_of< world::IDebugOverlay >().findAllOf(false);
			auto overlayType = overlayTypes[index - 1];

			Ref< world::IDebugOverlay > overlay = mandatory_non_null_type_cast< world::IDebugOverlay* >(overlayType->createInstance());
			if (overlay->create(m_context->getResourceManager()))
				m_renderControl->setDebugOverlay(overlay);
			else
			{
				m_renderControl->setDebugOverlay(nullptr);
				m_toolDebugOverlay->select(0);
			}
		}
		else
			m_renderControl->setDebugOverlay(nullptr);
	}

	if (updateQuality)
	{
		m_renderControl->setQuality(
			(world::Quality)getChecked(m_menuPostProcess)->getCommand().getId(),
			(world::Quality)getChecked(m_menuShadows)->getCommand().getId(),
			(world::Quality)getChecked(m_menuReflections)->getCommand().getId(),
			(world::Quality)getChecked(m_menuMotionBlur)->getCommand().getId(),
			(world::Quality)getChecked(m_menuAO)->getCommand().getId(),
			(world::Quality)getChecked(m_menuAA)->getCommand().getId()
		);
	}

	m_context->enqueueRedraw(this);
}

}
