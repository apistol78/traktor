/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/CameraRenderControl.h"
#include "Scene/Editor/DebugRenderControl.h"
#include "Scene/Editor/DefaultRenderControl.h"
#include "Scene/Editor/OrthogonalRenderControl.h"
#include "Scene/Editor/PerspectiveRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/MenuItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Custom/ToolBar/ToolBarDropMenu.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{
		
ui::MenuItem* getChecked(ui::MenuItem* menu)
{
	for (int i = 0; i < menu->count(); ++i)
	{
		if (menu->get(i)->isChecked())
			return menu->get(i);
	}
	return 0;
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
	T_ASSERT (m_context);

	m_cameraId = cameraId;
	m_viewId = viewId;

	Ref< const PropertyGroup > settings = context->getEditor()->getSettings();
	T_ASSERT (settings);

	int32_t viewType = settings->getProperty< int32_t >(L"SceneEditor.View" + toString(m_viewId), 0);
	bool gridEnable = settings->getProperty< bool >(L"Scene.Editor.GridEnable" + toString(m_viewId), true);
	bool guideEnable = settings->getProperty< bool >(L"Scene.Editor.GuideEnable" + toString(m_viewId), true);

	m_container = new ui::Container();
	if (!m_container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBar = new ui::custom::ToolBar();
	if (!m_toolBar->create(m_container))
		return false;

	m_toolBar->addImage(new ui::StyleBitmap(L"Scene.ToggleGrid"), 1);
	m_toolBar->addImage(new ui::StyleBitmap(L"Scene.ToggleGuide"), 1);

	m_toolView = new ui::custom::ToolBarDropDown(ui::Command(L"Scene.Editor.View"), ui::dpi96(100), i18n::Text(L"SCENE_EDITOR_VIEW_MODE"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_PERSPECTIVE"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_FRONT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_BACK"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_TOP"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_BOTTOM"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_LEFT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_RIGHT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_DEBUG"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_CAMERA"));
	m_toolView->select(viewType);

	m_toolToggleGrid = new ui::custom::ToolBarButton(
		i18n::Text(L"SCENE_EDITOR_TOGGLE_GRID"),
		0,
		ui::Command(1, L"Scene.Editor.ToggleGrid"),
		gridEnable ? ui::custom::ToolBarButton::BsDefaultToggled : ui::custom::ToolBarButton::BsDefaultToggle
	);

	m_toolToggleGuide = new ui::custom::ToolBarButton(
		i18n::Text(L"SCENE_EDITOR_TOGGLE_GUIDE"),
		1,
		ui::Command(1, L"Scene.Editor.ToggleGuide"),
		guideEnable ? ui::custom::ToolBarButton::BsDefaultToggled : ui::custom::ToolBarButton::BsDefaultToggle
	);

	m_toolAspect = new ui::custom::ToolBarDropDown(ui::Command(1, L"Scene.Editor.Aspect"), ui::dpi96(130), i18n::Text(L"SCENE_EDITOR_ASPECT"));
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

	Ref< ui::custom::ToolBarDropMenu > toolQualityMenu = new ui::custom::ToolBarDropMenu(ui::dpi96(130), i18n::Text(L"SCENE_EDITOR_QUALITY"), true, i18n::Text(L"SCENE_EDITOR_QUALITY_TOOLTIP"));

	m_menuPostProcess = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_POST_PROCESS"));
	m_menuPostProcess->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.PostProcessQuality"), L"Disabled", true, 0));
	m_menuPostProcess->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.PostProcessQuality"), L"Low", true, 0));
	m_menuPostProcess->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.PostProcessQuality"), L"Medium", true, 0));
	m_menuPostProcess->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.PostProcessQuality"), L"High", true, 0));
	m_menuPostProcess->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.PostProcessQuality"), L"Ultra", true, 0));
	toolQualityMenu->add(m_menuPostProcess);

	m_menuMotionBlur = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_MOTION_BLUR"));
	m_menuMotionBlur->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.MotionBlurQuality"), L"Disabled", true, 0));
	m_menuMotionBlur->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.MotionBlurQuality"), L"Low", true, 0));
	m_menuMotionBlur->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.MotionBlurQuality"), L"Medium", true, 0));
	m_menuMotionBlur->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.MotionBlurQuality"), L"High", true, 0));
	m_menuMotionBlur->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.MotionBlurQuality"), L"Ultra", true, 0));
	toolQualityMenu->add(m_menuMotionBlur);

	m_menuShadows = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_SHADOWS"));
	m_menuShadows->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.ShadowQuality"), L"Disabled", true, 0));
	m_menuShadows->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.ShadowQuality"), L"Low", true, 0));
	m_menuShadows->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.ShadowQuality"), L"Medium", true, 0));
	m_menuShadows->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.ShadowQuality"), L"High", true, 0));
	m_menuShadows->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.ShadowQuality"), L"Ultra", true, 0));
	toolQualityMenu->add(m_menuShadows);

	m_menuReflections = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_REFLECTIONS"));
	m_menuReflections->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.ReflectionsQuality"), L"Disabled", true, 0));
	m_menuReflections->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.ReflectionsQuality"), L"Low", true, 0));
	m_menuReflections->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.ReflectionsQuality"), L"Medium", true, 0));
	m_menuReflections->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.ReflectionsQuality"), L"High", true, 0));
	m_menuReflections->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.ReflectionsQuality"), L"Ultra", true, 0));
	toolQualityMenu->add(m_menuReflections);

	m_menuAO = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_AO"));
	m_menuAO->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.AmbientOcclusionQuality"), L"Disabled", true, 0));
	m_menuAO->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.AmbientOcclusionQuality"), L"Low", true, 0));
	m_menuAO->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.AmbientOcclusionQuality"), L"Medium", true, 0));
	m_menuAO->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.AmbientOcclusionQuality"), L"High", true, 0));
	m_menuAO->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.AmbientOcclusionQuality"), L"Ultra", true, 0));
	toolQualityMenu->add(m_menuAO);

	m_menuAA = new ui::MenuItem(i18n::Text(L"SCENE_EDITOR_AA"));
	m_menuAA->add(new ui::MenuItem(ui::Command(0, L"Scene.Editor.AntiAliasQuality"), L"Disabled", true, 0));
	m_menuAA->add(new ui::MenuItem(ui::Command(1, L"Scene.Editor.AntiAliasQuality"), L"Low", true, 0));
	m_menuAA->add(new ui::MenuItem(ui::Command(2, L"Scene.Editor.AntiAliasQuality"), L"Medium", true, 0));
	m_menuAA->add(new ui::MenuItem(ui::Command(3, L"Scene.Editor.AntiAliasQuality"), L"High", true, 0));
	m_menuAA->add(new ui::MenuItem(ui::Command(4, L"Scene.Editor.AntiAliasQuality"), L"Ultra", true, 0));
	toolQualityMenu->add(m_menuAA);

	m_toolBar->addItem(m_toolView);
	m_toolBar->addItem(m_toolToggleGrid);
	m_toolBar->addItem(m_toolToggleGuide);
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(m_toolAspect);
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(toolQualityMenu);
	m_toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &DefaultRenderControl::eventToolClick);

	m_menuPostProcess->get(0)->setChecked(true);
	m_menuMotionBlur->get(0)->setChecked(true);
	m_menuShadows->get(0)->setChecked(true);
	m_menuReflections->get(0)->setChecked(true);
	m_menuAO->get(0)->setChecked(true);
	m_menuAA->get(0)->setChecked(true);

	if (!createRenderControl(viewType))
		return false;

	return true;
}

void DefaultRenderControl::destroy()
{
	Ref< PropertyGroup > settings = m_context->getEditor()->checkoutGlobalSettings();
	T_ASSERT (settings);

	settings->setProperty< PropertyBoolean >(L"Scene.Editor.GridEnable" + toString(m_viewId), m_toolToggleGrid->isToggled());
	settings->setProperty< PropertyBoolean >(L"Scene.Editor.GuideEnable" + toString(m_viewId), m_toolToggleGuide->isToggled());

	m_context->getEditor()->commitGlobalSettings();
	m_toolView = 0;

	if (m_renderControl)
	{
		m_renderControl->destroy();
		m_renderControl = 0;
	}

	if (m_container)
	{
		m_container->destroy();
		m_container = 0;
	}
}

void DefaultRenderControl::updateWorldRenderer()
{
	if (m_renderControl)
		m_renderControl->updateWorldRenderer();
}

void DefaultRenderControl::setAspect(float aspect)
{
	if (m_renderControl)
		m_renderControl->setAspect(aspect);
}

void DefaultRenderControl::setQuality(world::Quality imageProcessQuality, world::Quality shadowQuality, world::Quality reflectionsQuality, world::Quality motionBlurQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality)
{
	if (m_renderControl)
		m_renderControl->setQuality(
			imageProcessQuality,
			shadowQuality,
			reflectionsQuality,
			motionBlurQuality,
			ambientOcclusionQuality,
			antiAliasQuality
		);
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
	if (m_renderControl)
		m_renderControl->update();
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

	switch (type)
	{
	case 0:
		{
			Ref< PerspectiveRenderControl > renderControl = new PerspectiveRenderControl();
			if (!renderControl->create(m_container, m_context, m_cameraId))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 1:	// Front
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveZ, m_cameraId))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 2:	// Back
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeZ, m_cameraId))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 3:	// Top
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveY, m_cameraId))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 4:	// Bottom
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeY, m_cameraId))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 5:	// Left
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::PositiveX, m_cameraId))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 6:	// Right
		{
			Ref< OrthogonalRenderControl > renderControl = new OrthogonalRenderControl();
			if (!renderControl->create(m_container, m_context, OrthogonalRenderControl::NegativeX, m_cameraId))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 7:	// Debug
		{
			Ref< DebugRenderControl > renderControl = new DebugRenderControl();
			if (!renderControl->create(m_container, m_context))
				return false;
			m_renderControl = renderControl;
		}
		break;

	case 8:	// Camera
		{
			Ref< CameraRenderControl > renderControl = new CameraRenderControl();
			if (!renderControl->create(m_container, m_context))
				return false;
			m_renderControl = renderControl;
		}
		break;
	}

	T_ASSERT (m_renderControl);

	m_container->update();
	m_renderControl->updateWorldRenderer();

	if (m_toolToggleGrid->isToggled())
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnableGrid"));
	else
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisableGrid"));

	if (m_toolToggleGuide->isToggled())
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.EnableGuide"));
	else
		m_renderControl->handleCommand(ui::Command(L"Scene.Editor.DisableGuide"));

	Ref< PropertyGroup > settings = m_context->getEditor()->checkoutGlobalSettings();
	T_ASSERT (settings);

	settings->setProperty< PropertyInteger >(L"SceneEditor.View" + toString(m_viewId), type);

	m_context->getEditor()->commitGlobalSettings();
	return true;
}

void DefaultRenderControl::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	bool updateQuality = false;

	if (event->getCommand() == L"Scene.Editor.View")
	{
		int32_t selected = m_toolView->getSelected();
		T_ASSERT (selected >= 0);
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
}

	}
}
