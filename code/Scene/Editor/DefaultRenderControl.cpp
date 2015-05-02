#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/DebugRenderControl.h"
#include "Scene/Editor/DefaultRenderControl.h"
#include "Scene/Editor/OrthogonalRenderControl.h"
#include "Scene/Editor/PerspectiveRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/MenuItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

// Resources
#include "Resources/EntityEdit.h"
#include "Resources/SceneEdit.h"

namespace traktor
{
	namespace scene
	{

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

	int32_t viewType = settings->getProperty< PropertyInteger >(L"SceneEditor.View" + toString(m_viewId), 0);
	bool gridEnable = settings->getProperty< PropertyBoolean >(L"Scene.Editor.GridEnable" + toString(m_viewId), true);
	bool guideEnable = settings->getProperty< PropertyBoolean >(L"Scene.Editor.GuideEnable" + toString(m_viewId), true);

	m_container = new ui::Container();
	if (!m_container->create(parent, ui::WsClientBorder, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolBar = new ui::custom::ToolBar();
	if (!m_toolBar->create(m_container))
		return false;

	m_toolBar->addImage(ui::Bitmap::load(c_ResourceSceneEdit, sizeof(c_ResourceSceneEdit), L"png"), 17);
	m_toolBar->addImage(ui::Bitmap::load(c_ResourceEntityEdit, sizeof(c_ResourceEntityEdit), L"png"), 5);

	m_toolView = new ui::custom::ToolBarDropDown(ui::Command(L"Scene.Editor.View"), 100, i18n::Text(L"SCENE_EDITOR_VIEW_MODE"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_PERSPECTIVE"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_FRONT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_BACK"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_TOP"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_BOTTOM"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_LEFT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_RIGHT"));
	m_toolView->add(i18n::Text(L"SCENE_EDITOR_VIEW_DEBUG"));
	m_toolView->select(viewType);

	m_toolToggleGrid = new ui::custom::ToolBarButton(
		i18n::Text(L"SCENE_EDITOR_TOGGLE_GRID"),
		16,
		ui::Command(1, L"Scene.Editor.ToggleGrid"),
		gridEnable ? ui::custom::ToolBarButton::BsDefaultToggled : ui::custom::ToolBarButton::BsDefaultToggle
	);

	m_toolToggleGuide = new ui::custom::ToolBarButton(
		i18n::Text(L"SCENE_EDITOR_TOGGLE_GUIDE"),
		5,
		ui::Command(1, L"Scene.Editor.ToggleGuide"),
		guideEnable ? ui::custom::ToolBarButton::BsDefaultToggled : ui::custom::ToolBarButton::BsDefaultToggle
	);

	m_toolToggleFollowEntity = new ui::custom::ToolBarButton(
		i18n::Text(L"SCENE_EDITOR_FOLLOW_ENTITY"),
		17 + 4,
		ui::Command(L"Scene.Editor.ToggleFollowEntity"),
		ui::custom::ToolBarButton::BsDefaultToggle
	);

	m_toolToggleLookAtEntity = new ui::custom::ToolBarButton(
		i18n::Text(L"SCENE_EDITOR_LOOK_AT_ENTITY"),
		17 + 3,
		ui::Command(L"Scene.Editor.ToggleLookAtEntity"),
		ui::custom::ToolBarButton::BsDefaultToggle
	);

	m_toolAspect = new ui::custom::ToolBarDropDown(ui::Command(1, L"Scene.Editor.Aspect"), 60, i18n::Text(L"SCENE_EDITOR_ASPECT"));
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

	m_toolPostProcess = new ui::custom::ToolBarDropDown(ui::Command(1, L"Scene.Editor.PostProcessQuality"), 80, i18n::Text(L"SCENE_EDITOR_POST_PROCESS"));
	m_toolPostProcess->add(L"Disabled");
	m_toolPostProcess->add(L"Low");
	m_toolPostProcess->add(L"Medium");
	m_toolPostProcess->add(L"High");
	m_toolPostProcess->add(L"Ultra");
	m_toolPostProcess->select(0);

	m_toolShadows = new ui::custom::ToolBarDropDown(ui::Command(1, L"Scene.Editor.ShadowQuality"), 80, i18n::Text(L"SCENE_EDITOR_SHADOWS"));
	m_toolShadows->add(L"Disabled");
	m_toolShadows->add(L"Low");
	m_toolShadows->add(L"Medium");
	m_toolShadows->add(L"High");
	m_toolShadows->add(L"Ultra");
	m_toolShadows->select(0);

	m_toolAO = new ui::custom::ToolBarDropDown(ui::Command(1, L"Scene.Editor.AmbientOcclusionQuality"), 80, i18n::Text(L"SCENE_EDITOR_AO"));
	m_toolAO->add(L"Disabled");
	m_toolAO->add(L"Low");
	m_toolAO->add(L"Medium");
	m_toolAO->add(L"High");
	m_toolAO->add(L"Ultra");
	m_toolAO->select(0);

	m_toolAA = new ui::custom::ToolBarDropDown(ui::Command(1, L"Scene.Editor.AntiAliasQuality"), 80, i18n::Text(L"SCENE_EDITOR_AA"));
	m_toolAA->add(L"Disabled");
	m_toolAA->add(L"Low");
	m_toolAA->add(L"Medium");
	m_toolAA->add(L"High");
	m_toolAA->add(L"Ultra");
	m_toolAA->select(0);

	m_toolBar->addItem(m_toolView);
	m_toolBar->addItem(m_toolToggleGrid);
	m_toolBar->addItem(m_toolToggleGuide);
	m_toolBar->addItem(m_toolToggleFollowEntity);
	m_toolBar->addItem(m_toolToggleLookAtEntity);
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(m_toolAspect);
	m_toolBar->addItem(new ui::custom::ToolBarSeparator());
	m_toolBar->addItem(m_toolPostProcess);
	m_toolBar->addItem(m_toolShadows);
	m_toolBar->addItem(m_toolAO);
	m_toolBar->addItem(m_toolAA);
	m_toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &DefaultRenderControl::eventToolClick);

	if (!createRenderControl(viewType))
		return false;

	Ref< Camera > camera = m_context->getCamera(m_cameraId);
	camera->setFollowEntity(0);
	camera->setLookAtEntity(0);

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

void DefaultRenderControl::setQuality(world::Quality postProcessQuality, world::Quality shadowQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality)
{
	if (m_renderControl)
		m_renderControl->setQuality(
			postProcessQuality,
			shadowQuality,
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
	else if (event->getCommand() == L"Scene.Editor.ToggleFollowEntity")
	{
		Ref< Camera > camera = m_context->getCamera(m_cameraId);
		if (m_toolToggleFollowEntity->isToggled())
		{
			RefArray< EntityAdapter > selectedEntities;
			if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
				camera->setFollowEntity(selectedEntities[0]);
		}
		else
			camera->setFollowEntity(0);
	}
	else if (event->getCommand() == L"Scene.Editor.ToggleLookAtEntity")
	{
		Ref< Camera > camera = m_context->getCamera(m_cameraId);
		if (m_toolToggleFollowEntity->isToggled())
		{
			RefArray< EntityAdapter > selectedEntities;
			if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
				camera->setLookAtEntity(selectedEntities[0]);
		}
		else
			camera->setLookAtEntity(0);
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
	else if (
		event->getCommand() == L"Scene.Editor.PostProcessQuality" ||
		event->getCommand() == L"Scene.Editor.ShadowQuality" ||
		event->getCommand() == L"Scene.Editor.AmbientOcclusionQuality" ||
		event->getCommand() == L"Scene.Editor.AntiAliasQuality"
	)
	{
		m_renderControl->setQuality(
			(world::Quality)m_toolPostProcess->getSelected(),
			(world::Quality)m_toolShadows->getSelected(),
			(world::Quality)m_toolAO->getSelected(),
			(world::Quality)m_toolAA->getSelected()
		);
	}
}

	}
}
