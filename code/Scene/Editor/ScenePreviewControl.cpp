#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "I18N/Text.h"
#include "I18N/Format.h"
#include "Physics/PhysicsManager.h"
#include "Scene/Scene.h"
#include "Scene/Editor/ScenePreviewControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/ISceneControllerEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/ISceneEditorPlugin.h"
#include "Scene/Editor/DefaultRenderControl.h"
#include "Scene/Editor/Modifiers/TranslateModifier.h"
#include "Scene/Editor/Modifiers/RotateModifier.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Scene/Editor/Events/FrameEvent.h"
#include "Scene/Editor/Events/ModifierChangedEvent.h"
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Slider.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/StatusBar/StatusBar.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/QuadSplitter.h"
#include "World/IEntityEventManager.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.ScenePreviewControl", ScenePreviewControl, ui::Widget)

ScenePreviewControl::ScenePreviewControl()
:	m_splitCount(StQuadruple)
,	m_lastDeltaTime(0.0f)
,	m_lastTime(0.0f)
,	m_lastPhysicsTime(0.0f)
{
}

bool ScenePreviewControl::create(ui::Widget* parent, SceneEditorContext* context)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	m_toolTogglePick = new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_TOGGLE_PICK"), 0, ui::Command(1, L"Scene.Editor.TogglePick"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleTranslate = new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_TRANSLATE"), 1, ui::Command(L"Scene.Editor.Translate"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleRotate = new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_ROTATE"), 2, ui::Command(L"Scene.Editor.Rotate"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleSnap = new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_TOGGLE_SNAP"), 3, ui::Command(1, L"Scene.Editor.ToggleSnap"), ui::custom::ToolBarButton::BsDefaultToggle);

	m_toolSnapSpacing = new ui::custom::ToolBarDropDown(ui::Command(L"Scene.Editor.SnapSpacing"), ui::scaleBySystemDPI(60), i18n::Text(L"SCENE_EDITOR_TOGGLE_SNAP_SPACING"));
	m_toolSnapSpacing->add(L"None");
	m_toolSnapSpacing->add(L"1/8");
	m_toolSnapSpacing->add(L"1/4");
	m_toolSnapSpacing->add(L"1/2");
	m_toolSnapSpacing->add(L"1");
	m_toolSnapSpacing->add(L"2");
	m_toolSnapSpacing->add(L"4");
	m_toolSnapSpacing->add(L"8");
	m_toolSnapSpacing->select(0);

	Ref< const PropertyGroup > settings = context->getEditor()->getSettings();
	T_ASSERT (settings);

	m_toolTogglePick->setToggled(settings->getProperty< PropertyBoolean >(L"SceneEditor.TogglePick", true));
	m_toolToggleTranslate->setToggled(true);
	m_toolToggleRotate->setToggled(false);
	m_toolToggleSnap->setToggled(settings->getProperty< PropertyBoolean >(L"SceneEditor.ToggleSnap", true));

	m_toolBarActions = new ui::custom::ToolBar();
	m_toolBarActions->create(this, ui::WsBorder);
	m_toolBarActions->addImage(new ui::StyleBitmap(L"Scene.Pointer"), 1);
	m_toolBarActions->addImage(new ui::StyleBitmap(L"Scene.Translate"), 1);
	m_toolBarActions->addImage(new ui::StyleBitmap(L"Scene.Rotate"), 1);
	m_toolBarActions->addImage(new ui::StyleBitmap(L"Scene.Snap"), 1);
	m_toolBarActions->addImage(new ui::StyleBitmap(L"Scene.Play"), 1);
	m_toolBarActions->addImage(new ui::StyleBitmap(L"Scene.Rewind"), 1);
	m_toolBarActions->addImage(new ui::StyleBitmap(L"Scene.Stop"), 1);
	m_toolBarActions->addImage(new ui::StyleBitmap(L"Scene.SingleView"), 1);
	m_toolBarActions->addImage(new ui::StyleBitmap(L"Scene.DualView"), 1);
	m_toolBarActions->addImage(new ui::StyleBitmap(L"Scene.QuadView"), 1);
	m_toolBarActions->addItem(m_toolTogglePick);
	m_toolBarActions->addItem(new ui::custom::ToolBarSeparator());
	m_toolBarActions->addItem(m_toolToggleTranslate);
	m_toolBarActions->addItem(m_toolToggleRotate);
	m_toolBarActions->addItem(new ui::custom::ToolBarSeparator());
	m_toolBarActions->addItem(m_toolToggleSnap);
	m_toolBarActions->addItem(m_toolSnapSpacing);
	m_toolBarActions->addItem(new ui::custom::ToolBarSeparator());
	m_toolBarActions->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_REWIND"), 5, ui::Command(L"Scene.Editor.Rewind")));
	m_toolBarActions->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_PLAY"), 4, ui::Command(L"Scene.Editor.Play")));
	m_toolBarActions->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_STOP"), 6, ui::Command(L"Scene.Editor.Stop")));
	m_toolBarActions->addItem(new ui::custom::ToolBarSeparator());
	m_toolBarActions->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_SINGLE_VIEW"), 7, ui::Command(L"Scene.Editor.SingleView")));
	m_toolBarActions->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_DOUBLE_VIEW"), 8, ui::Command(L"Scene.Editor.DoubleView")));
	m_toolBarActions->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_QUADRUPLE_VIEW"), 9, ui::Command(L"Scene.Editor.QuadrupleView")));
	m_toolBarActions->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ScenePreviewControl::eventToolBarActionClicked);

	// Let plugins create additional toolbar items.
	const RefArray< ISceneEditorPlugin >& editorPlugins = context->getEditorPlugins();
	for (RefArray< ISceneEditorPlugin >::const_iterator i = editorPlugins.begin(); i != editorPlugins.end(); ++i)
		(*i)->create(this, m_toolBarActions);

	m_context = context;
	m_context->addEventHandler< ModifierChangedEvent >(this, &ScenePreviewControl::eventModifierChanged);

	// Create modifiers.
	m_modifierTranslate = new TranslateModifier(m_context);
	m_modifierRotate = new RotateModifier(m_context);
	m_context->setModifier(m_modifierTranslate);

	m_splitCount = settings->getProperty< PropertyInteger >(L"SceneEditor.SplitCount", 4);

	if (!updateRenderControls())
		return false;

	updateEditState();

	m_idleEventHandler = ui::Application::getInstance()->addEventHandler< ui::IdleEvent >(this, &ScenePreviewControl::eventIdle);

	m_timer.start();
	return true;
}

void ScenePreviewControl::destroy()
{
	ui::Application::getInstance()->removeEventHandler< ui::IdleEvent >(m_idleEventHandler);

	// Save editor configuration.
	Ref< PropertyGroup > settings = m_context->getEditor()->checkoutGlobalSettings();
	T_ASSERT (settings);

	settings->setProperty< PropertyBoolean >(L"SceneEditor.TogglePick", m_toolTogglePick->isToggled());
	settings->setProperty< PropertyBoolean >(L"SceneEditor.ToggleSnap", m_toolToggleSnap->isToggled());
	settings->setProperty< PropertyInteger >(L"SceneEditor.SplitCount", m_splitCount);

	m_context->getEditor()->commitGlobalSettings();

	// Destroy render controls.
	for (RefArray< ISceneRenderControl >::iterator i = m_renderControls.begin(); i != m_renderControls.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
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

	if (command == L"Scene.Editor.SnapSpacing")
	{
		result = true;
	}
	else if (command == L"Scene.Editor.IncreaseSnap")
	{
		int32_t selected = m_toolSnapSpacing->getSelected();
		if (selected < m_toolSnapSpacing->count() - 1)
			m_toolSnapSpacing->select(selected + 1);
	}
	else if (command == L"Scene.Editor.DecreaseSnap")
	{
		int32_t selected = m_toolSnapSpacing->getSelected();
		if (selected > 0)
			m_toolSnapSpacing->select(selected - 1);
	}
	else if (command == L"Scene.Editor.Translate")
	{
		m_context->setModifier(m_modifierTranslate);
	}
	else if (command == L"Scene.Editor.Rotate")
	{
		m_context->setModifier(m_modifierRotate);
	}
	else if (command == L"Scene.Editor.TogglePick")
	{
		if (command.getId() == 0)
			m_toolTogglePick->setToggled(!m_toolTogglePick->isToggled());
	}
	else if (command == L"Scene.Editor.ToggleSnap")
	{
		if (command.getId() == 0)
			m_toolToggleSnap->setToggled(!m_toolToggleSnap->isToggled());
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
		m_splitCount = StSingle;
		updateRenderControls();
	}
	else if (command == L"Scene.Editor.DoubleView")
	{
		m_splitCount = StDouble;
		updateRenderControls();
	}
	else if (command == L"Scene.Editor.QuadrupleView")
	{
		m_splitCount = StQuadruple;
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
		if (!result)
		{
			for (RefArray< ISceneRenderControl >::iterator i = m_renderControls.begin(); i != m_renderControls.end(); ++i)
			{
				result = (*i)->handleCommand(command);
				if (result)
					break;
			}
		}

		// Propagate command to active modifier.
		if (!result)
		{
			IModifier* modifier = m_context->getModifier();
			if (modifier)
				result = modifier->handleCommand(command);
		}

		// Update settings in all entity editors.
		if (command == L"Editor.SettingsChanged")
		{
			RefArray< EntityAdapter > entities;
			m_context->getEntities(entities, SceneEditorContext::GfDescendants);

			for (RefArray< EntityAdapter >::iterator i = entities.begin(); i != entities.end(); ++i)
			{
				Ref< IEntityEditor > entityEditor = (*i)->getEntityEditor();
				if (entityEditor)
					entityEditor->handleCommand(command);
			}
		}
		// Propagate command to selected entity editors if render control has focus.
		else if (!result && containFocus())
		{
			RefArray< EntityAdapter > entities;
			m_context->getEntities(entities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);

			for (RefArray< EntityAdapter >::iterator i = entities.begin(); i != entities.end(); ++i)
			{
				Ref< IEntityEditor > entityEditor = (*i)->getEntityEditor();
				if (entityEditor)
				{
					result = entityEditor->handleCommand(command);
					if (result)
						break;
				}
			}
		}
	}

	if (result)
		updateEditState();

	return result;
}

bool ScenePreviewControl::getViewIndex(const ui::Point& position, uint32_t& outIndex) const
{
	for (RefArray< ISceneRenderControl >::const_iterator i = m_renderControls.begin(); i != m_renderControls.end(); ++i)
	{
		if ((*i)->hitTest(position))
		{
			outIndex = std::distance(m_renderControls.begin(), i);
			return true;
		}
	}
	return false;
}

ui::Size ScenePreviewControl::getPreferedSize() const
{
	return ui::Size(256, 256);
}

bool ScenePreviewControl::updateRenderControls()
{
	for (RefArray< ISceneRenderControl >::iterator i = m_renderControls.begin(); i != m_renderControls.end(); ++i)
		(*i)->destroy();

	m_renderControls.resize(0);

	safeDestroy(m_splitterRenderControls);

	if (m_splitCount == StSingle)
	{
		Ref< DefaultRenderControl > renderControl = new DefaultRenderControl();
		if (!renderControl->create(
			this,
			m_context,
			0,
			0
		))
			return false;

		m_renderControls.resize(1);
		m_renderControls[0] = renderControl;
	}
	else if (m_splitCount == StDouble)
	{
		Ref< ui::custom::Splitter > doubleSplitter = new ui::custom::Splitter();
		doubleSplitter->create(this, true, 50, true);

		m_renderControls.resize(2);
		for (int i = 0; i < 2; ++i)
		{
			Ref< DefaultRenderControl > renderControl = new DefaultRenderControl();
			if (!renderControl->create(
				doubleSplitter,
				m_context,
				i,
				1 + i
			))
				return false;

			m_renderControls[i] = renderControl;
		}

		m_splitterRenderControls = doubleSplitter;
	}
	else if (m_splitCount == StQuadruple)
	{
		Ref< ui::custom::QuadSplitter > quadSplitter = new ui::custom::QuadSplitter();
		quadSplitter->create(this, ui::Point(50, 50), true);

		m_renderControls.resize(4);
		for (int i = 0; i < 4; ++i)
		{
			Ref< DefaultRenderControl > renderControl = new DefaultRenderControl();
			if (!renderControl->create(
				quadSplitter,
				m_context,
				i,
				3 + i
			))
				return false;

			m_renderControls[i] = renderControl;
		}

		m_splitterRenderControls = quadSplitter;
	}

	update();
	updateWorldRenderer();

	return true;
}

void ScenePreviewControl::updateEditState()
{
	// Enable picking.
	m_context->setPickEnable(m_toolTogglePick->isToggled());

	// Snap mode.
	if (m_toolToggleSnap->isToggled())
		m_context->setSnapMode(SceneEditorContext::SmNeighbour);
	else
	{
		const float c_snapSpacing[] = { 0.0f, 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f };
		if (m_toolSnapSpacing->getSelected() >= 1)
		{
			m_context->setSnapSpacing(c_snapSpacing[m_toolSnapSpacing->getSelected()]);
			m_context->setSnapMode(SceneEditorContext::SmGrid);
		}
		else
			m_context->setSnapMode(SceneEditorContext::SmNone);
	}

	// Ensure toolbar is up-to-date.
	m_toolBarActions->update();
}

void ScenePreviewControl::eventToolBarActionClicked(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	handleCommand(command);
}

void ScenePreviewControl::eventModifierChanged(ModifierChangedEvent* event)
{
	m_toolToggleTranslate->setToggled(m_context->getModifier() == m_modifierTranslate);
	m_toolToggleRotate->setToggled(m_context->getModifier() == m_modifierRotate);
	m_toolBarActions->update();
}

void ScenePreviewControl::eventIdle(ui::IdleEvent* event)
{
	if (isVisible(true))
	{
		Ref< Scene > scene = m_context->getScene();

		// Filter delta time.
		float deltaTime = float(m_timer.getDeltaTime());
		deltaTime = std::min(deltaTime, 1.0f / 10.0f);
		deltaTime = float(deltaTime * 0.2f + m_lastDeltaTime * 0.8f);
		m_lastDeltaTime = deltaTime;

		float scaledTime = m_context->getTime();
		float scaledDeltaTime = m_context->isPlaying() ? deltaTime * m_context->getTimeScale() : 0.0f;

		// Update scene controller editor.
		Ref< ISceneControllerEditor > controllerEditor = m_context->getControllerEditor();
		if (controllerEditor)
			controllerEditor->update();

		// Use physics; update in steps of 1/60th of a second.
		if (m_context->getPhysicsEnable())
		{
			if (m_lastPhysicsTime > scaledTime)
				m_lastPhysicsTime = scaledTime;

			const float c_updateDeltaTime = 1.0f / 60.0f;

			while (m_lastPhysicsTime < scaledTime)
			{
				world::UpdateParams update;
				update.totalTime = m_lastPhysicsTime;
				update.deltaTime = c_updateDeltaTime;
				update.alternateTime = m_lastPhysicsTime;

				scene->update(update, true, true);
				m_context->getEntityEventManager()->update(update);
				m_context->getPhysicsManager()->update(1.0f / 60.0f, false);

				m_lastPhysicsTime += c_updateDeltaTime;
			}
		}
		else if (scene)
		{
			world::UpdateParams update;
			update.totalTime = scaledTime;
			update.deltaTime = scaledDeltaTime;
			update.alternateTime = scaledTime;

			scene->update(update, true, true);
			m_context->getEntityEventManager()->update(update);
		}

		// Issue updates on render controls.
		for (RefArray< ISceneRenderControl >::iterator i = m_renderControls.begin(); i != m_renderControls.end(); ++i)
			(*i)->update();

		// Update modifiers as selected entity might have moved.
		if (m_lastTime != scaledTime)
		{
			if (m_context->getModifier())
				m_context->getModifier()->selectionChanged();

			m_lastTime = scaledTime;
		}

		// Issue frame handlers.
		FrameEvent eventFrame(this);
		m_context->raisePostFrame(&eventFrame);

		// Update context time.
		m_context->setTime(scaledTime + scaledDeltaTime);

		event->requestMore();
	}
}

	}
}
