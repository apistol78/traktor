#include <sstream>
#include <limits>
#include "Scene/Editor/ScenePreviewControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/ISceneEditorPlugin.h"
#include "Scene/Editor/DefaultRenderControl.h"
#include "Scene/Editor/Modifiers/TranslateModifier.h"
#include "Scene/Editor/Modifiers/RotateModifier.h"
#include "Scene/Editor/FrameEvent.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/ISceneController.h"
#include "Scene/Scene.h"
#include "Physics/PhysicsManager.h"
#include "Editor/IEditor.h"
#include "Editor/Settings.h"
#include "Editor/TypeBrowseFilter.h"
#include "Database/Instance.h"
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/TableLayout.h"
#include "Ui/Bitmap.h"
#include "Ui/Slider.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/IdleEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/StatusBar/StatusBar.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/QuadSplitter.h"
#include "I18N/Text.h"
#include "I18N/Format.h"
#include "World/Entity/EntityUpdate.h"
#include "World/Entity/Entity.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

// Resources
#include "Resources/SceneEdit.h"
#include "Resources/Playback.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.ScenePreviewControl", ScenePreviewControl, ui::Widget)

ScenePreviewControl::ScenePreviewControl()
:	m_splitType(StQuadruple)
,	m_lastDeltaTime(0.0f)
,	m_lastPhysicsTime(0.0f)
{
}

bool ScenePreviewControl::create(ui::Widget* parent, SceneEditorContext* context)
{
	if (!ui::Container::create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolTogglePick = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_PICK"), ui::Command(1, L"Scene.Editor.TogglePick"), 10, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleX = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_X"), ui::Command(1, L"Scene.Editor.ToggleX"), 2, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleY = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_Y"), ui::Command(1, L"Scene.Editor.ToggleY"), 3, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleZ = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_Z"), ui::Command(1, L"Scene.Editor.ToggleZ"), 4, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleGrid = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_GRID"), ui::Command(1, L"Scene.Editor.ToggleGrid"), 16, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleGuide = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_GUIDE"), ui::Command(1, L"Scene.Editor.ToggleGuide"), 5, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleSnap = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_SNAP"), ui::Command(1, L"Scene.Editor.ToggleSnap"), 7, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleAddReference = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_ADD_REFERENCE"), ui::Command(1, L"Scene.Editor.ToggleAddReference"), 12, ui::custom::ToolBarButton::BsDefaultToggle);

	Ref< editor::Settings > settings = context->getEditor()->getSettings();
	T_ASSERT (settings);

	m_toolTogglePick->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.TogglePick", true));
	m_toolToggleX->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleX", true));
	m_toolToggleY->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleY", true));
	m_toolToggleZ->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleZ", true));
	m_toolToggleGrid->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleGrid", true));
	m_toolToggleGuide->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleGuide", true));
	m_toolToggleSnap->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleSnap", true));

	m_toolBarActions = gc_new< ui::custom::ToolBar >();
	m_toolBarActions->create(this, ui::WsBorder);
	m_toolBarActions->addImage(ui::Bitmap::load(c_ResourceSceneEdit, sizeof(c_ResourceSceneEdit), L"png"), 17);
	m_toolBarActions->addImage(ui::Bitmap::load(c_ResourcePlayback, sizeof(c_ResourcePlayback), L"png"), 6);
	m_toolBarActions->addItem(m_toolTogglePick);
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TRANSLATE"), ui::Command(L"Scene.Editor.Translate"), 0));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_ROTATE"), ui::Command(L"Scene.Editor.Rotate"), 1));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarActions->addItem(m_toolToggleX);
	m_toolBarActions->addItem(m_toolToggleY);
	m_toolBarActions->addItem(m_toolToggleZ);
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarActions->addItem(m_toolToggleGrid);
	m_toolBarActions->addItem(m_toolToggleGuide);
	m_toolBarActions->addItem(m_toolToggleSnap);
	m_toolBarActions->addItem(m_toolToggleAddReference);
	m_toolBarActions->addClickEventHandler(ui::createMethodHandler(this, &ScenePreviewControl::eventToolBarActionClicked));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_REWIND"), ui::Command(L"Scene.Editor.Rewind"), 17));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_PLAY"), ui::Command(L"Scene.Editor.Play"), 18));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_STOP"), ui::Command(L"Scene.Editor.Stop"), 19));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_SINGLE_VIEW"), ui::Command(L"Scene.Editor.SingleView"), 13));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_DOUBLE_VIEW"), ui::Command(L"Scene.Editor.DoubleView"), 14));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_QUADRUPLE_VIEW"), ui::Command(L"Scene.Editor.QuadrupleView"), 15));

	// Let plugins create additional toolbar items.
	const RefArray< ISceneEditorPlugin >& editorPlugins = context->getEditorPlugins();
	for (RefArray< ISceneEditorPlugin >::const_iterator i = editorPlugins.begin(); i != editorPlugins.end(); ++i)
		(*i)->create(this, m_toolBarActions);

	m_modifierTranslate = gc_new< TranslateModifier >();
	m_modifierRotate = gc_new< RotateModifier >();

	m_context = context;
	m_context->setModifier(m_modifierTranslate);

	updateRenderControls();
	updateEditState();

	// Register our event handler in case of message idle.
	m_idleHandler = ui::createMethodHandler(this, &ScenePreviewControl::eventIdle);
	ui::Application::getInstance().addEventHandler(ui::EiIdle, m_idleHandler);

	m_timer.start();
	return true;
}

void ScenePreviewControl::destroy()
{
	// Remove our idle handler first.
	if (m_idleHandler)
	{
		ui::Application::getInstance().removeEventHandler(ui::EiIdle, m_idleHandler);
		m_idleHandler = 0;
	}

	// Save editor configuration.
	Ref< editor::Settings > settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.TogglePick", m_toolTogglePick->isToggled());
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleX", m_toolToggleX->isToggled());
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleY", m_toolToggleY->isToggled());
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleZ", m_toolToggleZ->isToggled());
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleGrid", m_toolToggleGrid->isToggled());
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleGuide", m_toolToggleGuide->isToggled());
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleSnap", m_toolToggleSnap->isToggled());

	// Destroy render controls.
	for (RefArray< ISceneRenderControl >::iterator i = m_renderControls.begin(); i != m_renderControls.end(); ++i)
		(*i)->destroy();
	m_renderControls.resize(0);

	// Destroy widgets.
	if (m_toolBarActions)
	{
		m_toolBarActions->destroy();
		m_toolBarActions = 0;
	}
	Widget::destroy();
}

void ScenePreviewControl::updateWorldRenderer()
{
	for (RefArray< ISceneRenderControl >::iterator i = m_renderControls.begin(); i != m_renderControls.end(); ++i)
		(*i)->updateWorldRenderer();
}

bool ScenePreviewControl::handleCommand(const ui::Command& command)
{
	bool result = true;

	if (command == L"Scene.Editor.Translate")
		m_context->setModifier(m_modifierTranslate);
	else if (command == L"Scene.Editor.Rotate")
		m_context->setModifier(m_modifierRotate);
	else if (command == L"Scene.Editor.TogglePick")
	{
		if (command.getId() == 0)
			m_toolTogglePick->setToggled(!m_toolTogglePick->isToggled());
		updateEditState();
	}
	else if (command == L"Scene.Editor.ToggleX" || command == L"Scene.Editor.ToggleY" || command == L"Scene.Editor.ToggleZ")
	{
		if (command.getId() == 0)
		{
			if (command == L"Scene.Editor.ToggleX")
				m_toolToggleX->setToggled(!m_toolToggleX->isToggled());
			else if (command == L"Scene.Editor.ToggleY")
				m_toolToggleY->setToggled(!m_toolToggleY->isToggled());
			else if (command == L"Scene.Editor.ToggleZ")
				m_toolToggleZ->setToggled(!m_toolToggleZ->isToggled());
		}
		updateEditState();
	}
	else if (command == L"Scene.Editor.ToggleGrid")
	{
		if (command.getId() == 0)
			m_toolToggleGrid->setToggled(!m_toolToggleGrid->isToggled());
		updateEditState();
	}
	else if (command == L"Scene.Editor.ToggleGuide")
	{
		if (command.getId() == 0)
			m_toolToggleGuide->setToggled(!m_toolToggleGuide->isToggled());
		updateEditState();
	}
	else if (command == L"Scene.Editor.ToggleSnap")
	{
		if (command.getId() == 0)
			m_toolToggleSnap->setToggled(!m_toolToggleSnap->isToggled());
		updateEditState();
	}
	else if (command == L"Scene.Editor.ToggleAddReference")
	{
		if (command.getId() == 0)
			m_toolToggleAddReference->setToggled(!m_toolToggleAddReference->isToggled());
		updateEditState();
	}
	else if (command == L"Scene.Editor.Rewind")
	{
		m_context->setTime(0.0f);
		m_context->setPlaying(false);
		m_context->setPhysicsEnable(false);
		m_context->buildEntities();
	}
	else if (command == L"Scene.Editor.Play")
	{
		m_context->setPlaying(true);
		m_context->setPhysicsEnable(true);
	}
	else if (command == L"Scene.Editor.Stop")
	{
		m_context->setPlaying(false);
		m_context->setPhysicsEnable(false);
	}
	else if (command == L"Scene.Editor.SingleView")
	{
		m_splitType = StSingle;
		updateRenderControls();
	}
	else if (command == L"Scene.Editor.DoubleView")
	{
		m_splitType = StDouble;
		updateRenderControls();
	}
	else if (command == L"Scene.Editor.QuadrupleView")
	{
		m_splitType = StQuadruple;
		updateRenderControls();
	}
	else
	{
		result = false;

		// Propagate command to plug-ins.
		const RefArray< ISceneEditorPlugin >& editorPlugins = m_context->getEditorPlugins();
		for (RefArray< ISceneEditorPlugin >::const_iterator i = editorPlugins.begin(); i != editorPlugins.end(); ++i)
		{
			result = (*i)->handleCommand(command);
			if (result)
				break;
		}

		// Propagate command to active render control.
		for (RefArray< ISceneRenderControl >::iterator i = m_renderControls.begin(); i != m_renderControls.end(); ++i)
		{
			if ((result = (*i)->handleCommand(command)) == true)
				break;
		}
	}

	return result;
}

ui::Size ScenePreviewControl::getPreferedSize() const
{
	return ui::Size(256, 256);
}

void ScenePreviewControl::updateRenderControls()
{
	for (RefArray< ISceneRenderControl >::iterator i = m_renderControls.begin(); i != m_renderControls.end(); ++i)
		(*i)->destroy();

	m_renderControls.resize(0);

	if (m_splitterRenderControls)
	{
		m_splitterRenderControls->destroy();
		m_splitterRenderControls = 0;
	}

	if (m_splitType == StSingle)
	{
		m_renderControls.resize(1);

		Ref< DefaultRenderControl > renderControl = gc_new< DefaultRenderControl >();
		if (renderControl->create(
			this,
			m_context,
			0
		))
			m_renderControls[0] = renderControl;
	}
	else if (m_splitType == StDouble)
	{
		Ref< ui::custom::Splitter > doubleSplitter = gc_new< ui::custom::Splitter >();
		doubleSplitter->create(this, true, 50, true);

		m_renderControls.resize(2);
		for (int i = 0; i < 2; ++i)
		{
			Ref< DefaultRenderControl > renderControl = gc_new< DefaultRenderControl >();
			if (renderControl->create(
				doubleSplitter,
				m_context,
				i
			))
				m_renderControls[i] = renderControl;
		}

		m_splitterRenderControls = doubleSplitter;
	}
	else if (m_splitType == StQuadruple)
	{
		Ref< ui::custom::QuadSplitter > quadSplitter = gc_new< ui::custom::QuadSplitter >();
		quadSplitter->create(this, ui::Point(50, 50), true);

		m_renderControls.resize(4);
		for (int i = 0; i < 4; ++i)
		{
			Ref< DefaultRenderControl > renderControl = gc_new< DefaultRenderControl >();
			if (renderControl->create(
				quadSplitter,
				m_context,
				i
			))
				m_renderControls[i] = renderControl;
		}

		m_splitterRenderControls = quadSplitter;
	}

	update();
	updateWorldRenderer();
}

void ScenePreviewControl::updateEditState()
{
	// Enable picking.
	m_context->setPickEnable(m_toolTogglePick->isToggled());

	// Enabled axis.
	uint32_t axisEnabled = 0;
	if (m_toolToggleX->isToggled())
		axisEnabled |= SceneEditorContext::AeX;
	if (m_toolToggleY->isToggled())
		axisEnabled |= SceneEditorContext::AeY;
	if (m_toolToggleZ->isToggled())
		axisEnabled |= SceneEditorContext::AeZ;
	m_context->setAxisEnable(axisEnabled);

	// Guides enabled.
	m_context->setGridEnable(m_toolToggleGrid->isToggled());
	m_context->setGuideEnable(m_toolToggleGuide->isToggled());
	m_context->setSnapEnable(m_toolToggleSnap->isToggled());
	m_context->setAddReferenceMode(m_toolToggleAddReference->isToggled());

	// Ensure toolbar is up-to-date.
	m_toolBarActions->update();
}

void ScenePreviewControl::eventToolBarActionClicked(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	handleCommand(command);
}

void ScenePreviewControl::eventIdle(ui::Event* event)
{
	ui::IdleEvent* idleEvent = checked_type_cast< ui::IdleEvent* >(event);
	if (isVisible(true))
	{
		// Filter delta time.
		float deltaTime = float(m_timer.getDeltaTime());
		deltaTime = std::min(deltaTime, 1.0f / 10.0f);
		deltaTime = float(deltaTime * 0.2f + m_lastDeltaTime * 0.8f);
		m_lastDeltaTime = deltaTime;

		float scaledTime = m_context->getTime();
		float scaledDeltaTime = m_context->isPlaying() ? deltaTime * m_context->getTimeScale() : 0.0f;

		// Update scene controller.
		Ref< Scene > scene = m_context->getScene();
		if (scene)
		{
			Ref< ISceneController > controller = scene->getController();
			if (controller)
			{
				controller->update(
					m_context->getScene(),
					scaledTime,
					scaledDeltaTime
				);
			}
		}

		// Update physics; update in steps of 1/60th of a second.
		if (m_context->getPhysicsEnable())
		{
			if (m_lastPhysicsTime > scaledTime)
				m_lastPhysicsTime = scaledTime;

			while (m_lastPhysicsTime < scaledTime)
			{
				m_context->getPhysicsManager()->update();
				m_lastPhysicsTime += 1.0f / 60.0f;
			}
		}

		// Update entities.
		Ref< EntityAdapter > rootEntityAdapter = m_context->getRootEntityAdapter();
		Ref< world::Entity > rootEntity = rootEntityAdapter ? rootEntityAdapter->getEntity() : 0;

		// Update entities.
		if (rootEntity)
		{
			world::EntityUpdate entityUpdate(deltaTime);
			rootEntity->update(&entityUpdate);
		}

		// Issue updates on render controls.
		for (RefArray< ISceneRenderControl >::iterator i = m_renderControls.begin(); i != m_renderControls.end(); ++i)
			(*i)->update();

		// Issue frame handlers.
		FrameEvent eventFrame(this);
		m_context->raisePostFrame(&eventFrame);

		// Update context time.
		m_context->setTime(scaledTime + scaledDeltaTime);

		idleEvent->requestMore();
	}
}

	}
}
