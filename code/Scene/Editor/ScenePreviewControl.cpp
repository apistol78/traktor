#include <sstream>
#include <limits>
#include "Scene/Editor/ScenePreviewControl.h"
#include "Scene/Editor/SceneRenderControl.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/Modifiers/TranslateModifier.h"
#include "Scene/Editor/Modifiers/RotateModifier.h"
#include "Scene/Editor/Modifiers/ScaleModifier.h"
#include "Scene/Editor/FrameEvent.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IEntityEditor.h"
#include "Physics/PhysicsManager.h"
#include "Editor/IEditor.h"
#include "Editor/Settings.h"
#include "Editor/TypeBrowseFilter.h"
#include "Database/Instance.h"
#include "Ui/Command.h"
#include "Ui/TableLayout.h"
#include "Ui/Bitmap.h"
#include "Ui/Slider.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/ToolBar/ToolBarEmbed.h"
#include "Ui/Custom/StatusBar/StatusBar.h"
#include "Ui/Custom/InputDialog.h"
#include "I18N/Text.h"
#include "I18N/Format.h"
#include "World/PostProcess/PostProcessSettings.h"
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
:	m_frameTime(std::numeric_limits< double >::max())
,	m_renderTime(std::numeric_limits< double >::max())
{
}

bool ScenePreviewControl::create(ui::Widget* parent, SceneEditorContext* context)
{
	if (!ui::Container::create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*,100%,*", 0, 0)))
		return false;

	m_toolTogglePick = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_PICK"), ui::Command(L"Scene.Editor.TogglePick"), 10, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleX = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_X"), ui::Command(L"Scene.Editor.ToggleX"), 2, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleY = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_Y"), ui::Command(L"Scene.Editor.ToggleY"), 3, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleZ = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_Z"), ui::Command(L"Scene.Editor.ToggleZ"), 4, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleGuide = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_GUIDE"), ui::Command(L"Scene.Editor.ToggleGuide"), 5, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleSnap = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_SNAP"), ui::Command(L"Scene.Editor.ToggleSnap"), 7, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleAddReference = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TOGGLE_ADD_REFERENCE"), ui::Command(L"Scene.Editor.ToggleAddReference"), 12, ui::custom::ToolBarButton::BsDefaultToggle);

	Ref< editor::Settings > settings = context->getEditor()->getSettings();
	T_ASSERT (settings);

	m_toolTogglePick->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.TogglePick", true));
	m_toolToggleX->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleX", true));
	m_toolToggleY->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleY", true));
	m_toolToggleZ->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleZ", true));
	m_toolToggleGuide->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleGuide", true));
	m_toolToggleSnap->setToggled(settings->getProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleSnap", true));

	m_toolBarActions = gc_new< ui::custom::ToolBar >();
	m_toolBarActions->create(this, ui::WsBorder);
	m_toolBarActions->addImage(ui::Bitmap::load(c_ResourceSceneEdit, sizeof(c_ResourceSceneEdit), L"png"), 13);
	m_toolBarActions->addImage(ui::Bitmap::load(c_ResourcePlayback, sizeof(c_ResourcePlayback), L"png"), 6);
	m_toolBarActions->addItem(m_toolTogglePick);
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_TRANSLATE"), ui::Command(L"Scene.Editor.Translate"), 0));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_ROTATE"), ui::Command(L"Scene.Editor.Rotate"), 1));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_SCALE"), ui::Command(L"Scene.Editor.Scale"), 11));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarActions->addItem(m_toolToggleX);
	m_toolBarActions->addItem(m_toolToggleY);
	m_toolBarActions->addItem(m_toolToggleZ);
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_EDIT_SPACE_WORLD"), ui::Command(L"Scene.Editor.EditSpaceWorld"), 8));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_EDIT_SPACE_OBJECT"), ui::Command(L"Scene.Editor.EditSpaceObject"), 9));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarActions->addItem(m_toolToggleGuide);
	m_toolBarActions->addItem(m_toolToggleSnap);
	m_toolBarActions->addItem(m_toolToggleAddReference);
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_BROWSE_POSTPROCESS"), ui::Command(L"Scene.Editor.BrowsePostProcess"), 6));
	m_toolBarActions->addClickEventHandler(ui::createMethodHandler(this, &ScenePreviewControl::eventToolBarActionClicked));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_REWIND"), ui::Command(L"Scene.Editor.Rewind"), 13));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_PLAY"), ui::Command(L"Scene.Editor.Play"), 14));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_STOP"), ui::Command(L"Scene.Editor.Stop"), 15));

	m_sliderTimeScale = gc_new< ui::Slider >();
	m_sliderTimeScale->create(m_toolBarActions);
	m_sliderTimeScale->setRange(0, 100);
	m_sliderTimeScale->setValue(100);
	m_sliderTimeScale->addChangeEventHandler(ui::createMethodHandler(this, &ScenePreviewControl::eventTimeScaleChanged));
	m_toolBarActions->addItem(gc_new< ui::custom::ToolBarEmbed >(m_sliderTimeScale, 100));

	// Let profiles create additional toolbar items.
	const RefArray< ISceneEditorProfile >& profiles = context->getEditorProfiles();
	for (RefArray< ISceneEditorProfile >::const_iterator i = profiles.begin(); i != profiles.end(); ++i)
		(*i)->createToolBarItems(m_toolBarActions);

	m_sceneRenderControl = gc_new< SceneRenderControl >();
	if (!m_sceneRenderControl->create(this, context))
		return false;

	m_infoContainer = gc_new< ui::Container >();
	m_infoContainer->create(this, ui::WsClientBorder, gc_new< ui::TableLayout >(L"100%", L"*", 2, 0));

	m_statusText = gc_new< ui::custom::StatusBar >();
	m_statusText->create(m_infoContainer, ui::WsDoubleBuffer);
	m_statusText->addDoubleClickEventHandler(ui::createMethodHandler(this, &ScenePreviewControl::eventCameraDoubleClick));

	m_modifierTranslate = gc_new< TranslateModifier >();
	m_modifierRotate = gc_new< RotateModifier >();
	m_modifierScale = gc_new< ScaleModifier >();

	m_context = context;
	m_context->setModifier(m_modifierTranslate);
	m_context->addPostBuildEventHandler(ui::createMethodHandler(this, &ScenePreviewControl::eventContextPostBuild));
	m_context->addPostFrameEventHandler(ui::createMethodHandler(this, &ScenePreviewControl::eventContextPostFrame));

	updateEditState();
	updateInformation();

	return true;
}

void ScenePreviewControl::destroy()
{
	// Save editor configuration.
	Ref< editor::Settings > settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.TogglePick", m_toolTogglePick->isToggled());
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleX", m_toolToggleX->isToggled());
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleY", m_toolToggleY->isToggled());
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleZ", m_toolToggleZ->isToggled());
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleGuide", m_toolToggleGuide->isToggled());
	settings->setProperty< editor::PropertyBoolean >(L"SceneEditor.ToggleSnap", m_toolToggleSnap->isToggled());

	// Destroy widgets.
	if (m_sceneRenderControl)
	{
		m_sceneRenderControl->destroy();
		m_sceneRenderControl = 0;
	}
	if (m_toolBarActions)
	{
		m_toolBarActions->destroy();
		m_toolBarActions = 0;
	}
	Widget::destroy();
}

void ScenePreviewControl::setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings)
{
	m_sceneRenderControl->setWorldRenderSettings(worldRenderSettings);
}

bool ScenePreviewControl::handleCommand(const ui::Command& command)
{
	bool result = true;

	if (command == L"Scene.Editor.Translate")
		m_context->setModifier(m_modifierTranslate);
	else if (command == L"Scene.Editor.Rotate")
		m_context->setModifier(m_modifierRotate);
	else if (command == L"Scene.Editor.Scale")
		m_context->setModifier(m_modifierScale);
	else if (command == L"Scene.Editor.TogglePick")
		updateEditState();
	else if (command == L"Scene.Editor.ToggleX" || command == L"Scene.Editor.ToggleY" || command == L"Scene.Editor.ToggleZ")
		updateEditState();
	else if (command == L"Scene.Editor.EditSpaceWorld")
		m_context->setEditSpace(SceneEditorContext::EsWorld);
	else if (command == L"Scene.Editor.EditSpaceObject")
		m_context->setEditSpace(SceneEditorContext::EsObject);
	else if (command == L"Scene.Editor.ToggleGuide")
		updateEditState();
	else if (command == L"Scene.Editor.ToggleSnap")
		updateEditState();
	else if (command == L"Scene.Editor.ToggleAddReference")
		updateEditState();
	else if (command == L"Scene.Editor.BrowsePostProcess")
	{
		editor::TypeBrowseFilter filter(type_of< world::PostProcessSettings >());
		Ref< db::Instance > postProcessInstance = m_context->getEditor()->browseInstance(&filter);
		if (postProcessInstance)
		{
			Ref< world::PostProcessSettings > postProcessSettings = postProcessInstance->getObject< world::PostProcessSettings >();
			if (postProcessSettings)
				m_sceneRenderControl->setPostProcessSettings(postProcessSettings);
		}
	}
	else if (command == L"Scene.Editor.Rewind")
	{
		m_context->setPhysicsEnable(false);
		m_context->resetEntities();
	}
	else if (command == L"Scene.Editor.Play")
		m_context->setPhysicsEnable(true);
	else if (command == L"Scene.Editor.Stop")
		m_context->setPhysicsEnable(false);
	else
	{
		// Propagate command to render control.
		result = m_sceneRenderControl->handleCommand(command);
	}

	return result;
}

ui::Size ScenePreviewControl::getPreferedSize() const
{
	return ui::Size(256, 256);
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
	m_context->setGuideEnable(m_toolToggleGuide->isToggled());
	m_context->setSnapEnable(m_toolToggleSnap->isToggled());
	m_context->setAddReferenceMode(m_toolToggleAddReference->isToggled());
}

void ScenePreviewControl::updateInformation()
{
	// See if selected entity has some special message to show.
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
	{
		std::wstring statusText;
		if (
			selectedEntities[0]->getEntityEditor() &&
			selectedEntities[0]->getEntityEditor()->getStatusText(m_context, selectedEntities[0], statusText)
		)
		{
			m_statusText->setText(statusText);
			m_infoContainer->update();
			return;
		}
	}

	// Nop, show default information.
	Ref< Camera > camera = m_context->getCamera();
	Vector4 cameraPosition = camera->getCurrentView().inverseOrtho().translation();

	uint32_t bodyCount = 0, activeBodyCount = 0;
	Ref< physics::PhysicsManager > physicsManager = m_context->getPhysicsManager();
	if (physicsManager)
		physicsManager->getBodyCount(bodyCount, activeBodyCount);

	StringOutputStream ss;
	ss << i18n::Format(L"SCENE_EDITOR_CAMERA_POSITION", float(cameraPosition.x()), float(cameraPosition.y()), float(cameraPosition.z()));
	ss << L" | ";
	ss << i18n::Format(L"SCENE_EDITOR_DELTA_SCALE", m_context->getDeltaScale());
	ss << L" | ";
	ss << i18n::Format(L"SCENE_EDITOR_FRAME_TIME", int32_t(m_frameTime * 1000.0), int32_t(1.0 / m_frameTime));
	ss << L" | ";
	ss << i18n::Format(L"SCENE_EDITOR_RENDER_TIME", int32_t(m_renderTime * 1000.0), int32_t(1.0 / m_renderTime));
	ss << L" | ";
	ss << i18n::Format(L"SCENE_EDITOR_PHYSICS", int32_t(bodyCount), int32_t(activeBodyCount));

	m_statusText->setText(ss.str());
}

void ScenePreviewControl::eventToolBarActionClicked(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	handleCommand(command);
}

void ScenePreviewControl::eventTimeScaleChanged(ui::Event* event)
{
	float timeScale = m_sliderTimeScale->getValue() / 100.0f;
	m_context->setTimeScale(timeScale);
}

void ScenePreviewControl::eventContextPostBuild(ui::Event* event)
{
	updateInformation();
}

void ScenePreviewControl::eventContextPostFrame(ui::Event* event)
{
	m_frameTime = checked_type_cast< const FrameEvent* >(event)->getFrameTime();
	m_renderTime = checked_type_cast< const FrameEvent* >(event)->getRenderTime();
	updateInformation();
}

void ScenePreviewControl::eventCameraDoubleClick(ui::Event* event)
{
	Ref< Camera > camera = m_context->getCamera();
	T_ASSERT (camera);

	const Vector4& position = camera->getTargetPosition();
	const Quaternion& orientation = camera->getTargetOrientation();

	float head, pitch, bank;
	orientation.toEulerAngles(head, pitch, bank);

	Ref< ui::NumericEditValidator > validator = gc_new< ui::NumericEditValidator >(true);
	ui::custom::InputDialog::Field fields[] =
	{
		{ i18n::Text(L"SCENE_EDITOR_CAMERA_X"), toString(position.x()), validator.getPtr() },
		{ i18n::Text(L"SCENE_EDITOR_CAMERA_Y"), toString(position.y()), validator.getPtr() },
		{ i18n::Text(L"SCENE_EDITOR_CAMERA_Z"), toString(position.z()), validator.getPtr() },
		{ i18n::Text(L"SCENE_EDITOR_CAMERA_HEAD"), toString(rad2deg(head)), validator.getPtr() },
		{ i18n::Text(L"SCENE_EDITOR_CAMERA_PITCH"), toString(rad2deg(pitch)), validator.getPtr() },
		{ i18n::Text(L"SCENE_EDITOR_CAMERA_BANK"), toString(rad2deg(bank)), validator.getPtr() }
	};

	ui::custom::InputDialog inputDialog;
	inputDialog.create(
		this,
		i18n::Text(L"SCENE_EDITOR_SET_CAMERA_TITLE"),
		i18n::Text(L"SCENE_EDITOR_SET_CAMERA_MESSAGE"),
		fields,
		sizeof_array(fields)
	);
	if (inputDialog.showModal() == ui::DrOk)
	{
		camera->setTargetPosition(Vector4(
			parseString< float >(fields[0].value),
			parseString< float >(fields[1].value),
			parseString< float >(fields[2].value),
			1.0f
		));
		camera->setTargetOrientation(Quaternion(
			deg2rad(parseString< float >(fields[3].value)),
			deg2rad(parseString< float >(fields[4].value)),
			deg2rad(parseString< float >(fields[5].value))
		));
	}
	inputDialog.destroy();	
}

	}
}
