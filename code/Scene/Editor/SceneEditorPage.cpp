#include "Scene/Editor/SceneEditorPage.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/ScenePreviewControl.h"
#include "Scene/Editor/SceneRenderControl.h"
#include "Scene/Editor/SceneEditorProfile.h"
#include "Scene/Editor/EntityClipboardData.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SelectEvent.h"
#include "Scene/Editor/Camera.h"
#include "Scene/SceneAsset.h"
#include "Editor/IEditor.h"
#include "Editor/Settings.h"
#include "Editor/UndoStack.h"
#include "Database/Database.h"
#include "Physics/PhysicsManager.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/EntityInstance.h"
#include "World/Entity/SpatialEntity.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalSpatialEntityData.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "Resource/IResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Bitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridDragEvent.h"
#include "Ui/Custom/BackgroundWorkerDialog.h"
#include "Ui/Custom/InputDialog.h"
#include "I18N/Text.h"
#include "Database/Instance.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Log/Log.h"

// Resources
#include "Resources/EntityEdit.h"
#include "Resources/EntityTypes.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const Guid c_guidWhiteRoomScene(L"{473467B0-835D-EF45-B308-E3C3C5B0F226}");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneEditorPage", SceneEditorPage, editor::IEditorPage)

SceneEditorPage::SceneEditorPage(SceneEditorContext* context)
:	m_context(context)
{
}

bool SceneEditorPage::create(ui::Container* parent)
{
	// Create editor panel.
	m_editPanel = gc_new< ui::Container >();
	m_editPanel->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"100%", 0, 0));

	m_editControl = gc_new< ScenePreviewControl >();
	m_editControl->create(m_editPanel, m_context);

	// Create entity panel.
	m_entityPanel = gc_new< ui::Container >();
	m_entityPanel->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*,100%", 0, 0));
	m_entityPanel->setText(i18n::Text(L"SCENE_EDITOR_ENTITIES"));

	m_entityMenu = gc_new< ui::PopupMenu >();
	m_entityMenu->create();
	m_entityMenu->add(gc_new< ui::MenuItem >(ui::Command(L"Scene.Editor.AddEntity"), i18n::Text(L"SCENE_EDITOR_ADD_ENTITY")));
	m_entityMenu->add(gc_new< ui::MenuItem >(ui::Command(L"Editor.Delete"), i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY")));

	m_toolLookAtEntity = gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_LOOK_AT_ENTITY"), ui::Command(L"Scene.Editor.LookAtEntity"), 3, ui::custom::ToolBarButton::BsDefaultToggle);

	m_entityToolBar = gc_new< ui::custom::ToolBar >();
	m_entityToolBar->create(m_entityPanel);
	m_entityToolBar->addImage(ui::Bitmap::load(c_ResourceEntityEdit, sizeof(c_ResourceEntityEdit), L"png"), 4);
	m_entityToolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY"), ui::Command(L"Editor.Delete"), 2));
	m_entityToolBar->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_entityToolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY"), ui::Command(L"Scene.Editor.MoveToSelectedEntity"), 0));
	m_entityToolBar->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"SCENE_EDITOR_MOVE_ENTITY_INTO_VIEW"), ui::Command(L"Scene.Editor.MoveSelectedEntityIntoView"), 1));
	m_entityToolBar->addItem(gc_new< ui::custom::ToolBarSeparator >());
	m_entityToolBar->addItem(m_toolLookAtEntity);
	m_entityToolBar->addClickEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventEntityToolClick));

	m_instanceGrid = gc_new< ui::custom::GridView >();
	m_instanceGrid->create(m_entityPanel, ui::custom::GridView::WsDrag | ui::WsDoubleBuffer);
	m_instanceGrid->addImage(ui::Bitmap::load(c_ResourceEntityTypes, sizeof(c_ResourceEntityTypes), L"png"), 4);
	m_instanceGrid->addColumn(gc_new< ui::custom::GridColumn >(i18n::Text(L"SCENE_EDITOR_ENTITY_NAME"), 300));
	m_instanceGrid->addSelectEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventInstanceSelect));
	m_instanceGrid->addButtonDownEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventInstanceButtonDown));
	m_instanceGrid->addDoubleClickEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventInstanceDoubleClick));

	m_instanceGridFontItalic = gc_new< ui::Font >(cref(m_instanceGrid->getFont()));
	m_instanceGridFontItalic->setItalic(true);

	m_instanceGridFontBold = gc_new< ui::Font >(cref(m_instanceGrid->getFont()));
	m_instanceGridFontBold->setBold(true);

	m_context->getEditor()->createAdditionalPanel(m_entityPanel, 250, false);

	// Context event handlers.
	m_context->addPostBuildEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextPostBuild));
	m_context->addSelectEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextSelect));
	m_context->addPreModifyEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextPreModify));

	// Restore camera from settings.
	Ref< editor::Settings > settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	Vector4 cameraPosition = settings->getProperty< editor::PropertyVector4 >(L"SceneEditor.CameraPosition", Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	Quaternion cameraOrientation = settings->getProperty< editor::PropertyQuaternion >(L"SceneEditor.CameraOrientation", Quaternion::identity());

	Ref< Camera > camera = gc_new< Camera >(cref(Matrix44::identity()));
	camera->setCurrentPosition(cameraPosition);
	camera->setTargetPosition(cameraPosition);
	camera->setCurrentOrientation(cameraOrientation);
	camera->setTargetOrientation(cameraOrientation);
	m_context->setCamera(camera);

	// Restore other settings.
	m_context->setDeltaScale(settings->getProperty< editor::PropertyFloat >(L"SceneEditor.DeltaScale", m_context->getDeltaScale()));

	m_undoStack = gc_new< editor::UndoStack >();

	return true;
}

void SceneEditorPage::destroy()
{
	// Save camera position in editor settings.
	Ref< editor::Settings > settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	settings->setProperty< editor::PropertyVector4 >(L"SceneEditor.CameraPosition", m_context->getCamera()->getCurrentPosition());
	settings->setProperty< editor::PropertyQuaternion >(L"SceneEditor.CameraOrientation", m_context->getCamera()->getCurrentOrientation());
	settings->setProperty< editor::PropertyFloat >(L"SceneEditor.DeltaScale", m_context->getDeltaScale());

	// Destroy entity panel.
	m_context->getEditor()->destroyAdditionalPanel(m_entityPanel);

	// Destroy widgets.
	m_entityMenu->destroy();
	m_entityPanel->destroy();
	m_editPanel->destroy();

	// Destroy physics manager.
	if (m_context->getPhysicsManager())
		m_context->getPhysicsManager()->destroy();
}

void SceneEditorPage::activate()
{
	m_editControl->setVisible(true);
	m_context->getEditor()->showAdditionalPanel(m_entityPanel);
}

void SceneEditorPage::deactivate()
{
	m_context->getEditor()->hideAdditionalPanel(m_entityPanel);
	m_editControl->setVisible(false);
}

bool SceneEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	EnterLeave scopeVisible(
		makeFunctor< ScenePreviewControl, bool >(m_editControl, &ScenePreviewControl::setVisible, false),
		makeFunctor< ScenePreviewControl, bool >(m_editControl, &ScenePreviewControl::setVisible, true)
	);

	Ref< SceneAsset > sceneAsset = dynamic_type_cast< SceneAsset* >(data);
	if (sceneAsset)
	{
		m_context->setSceneAsset(sceneAsset);

		updateScene();
		updateInstanceGrid();
	}
	else
	{
		Ref< world::EntityData > entityData = dynamic_type_cast< world::EntityData* >(data);
		if (!entityData)
			return false;

		sceneAsset = createWhiteRoomSceneAsset(entityData);
		if (!sceneAsset)
			return false;

		m_context->setSceneAsset(sceneAsset);
		updateScene();

		m_context->setCamera(gc_new< Camera >(cref(translate(
			0.0f,
			0.0f,
			-4.0f
		))));

		updateInstanceGrid();
	}

	m_dataObject = checked_type_cast< Serializable* >(data);
	updatePropertyObject();

	return true;
}

Object* SceneEditorPage::getDataObject()
{
	return m_dataObject;
}

void SceneEditorPage::propertiesChanged()
{
	updateScene();
	updateInstanceGrid();
}

bool SceneEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	const Type* primaryType = instance->getPrimaryType();
	T_ASSERT (primaryType);

	if (is_type_of< world::EntityData >(*primaryType))
	{
		Ref< EntityAdapter > parentGroupAdapter;

		// Get selected items, must be a single item.
		RefArray< ui::custom::GridRow > selectedRows;
		if (m_instanceGrid->getRows(selectedRows, ui::custom::GridView::GfDescendants | ui::custom::GridView::GfSelectedOnly) == 1)
		{
			Ref< EntityAdapter > selectedEntity = selectedRows[0]->getData< EntityAdapter >(L"ENTITY");
			T_ASSERT (selectedEntity);

			parentGroupAdapter = selectedEntity->getParentGroup();
			if (!parentGroupAdapter)
					return false;
		}
		else if (m_context->getSceneAsset()->getInstance())
		{
			parentGroupAdapter = m_context->getRootEntityAdapter();
			if (!parentGroupAdapter)
					return false;
		}

		Ref< world::EntityData > entityData;

		// Create external reference to entity data.
		if (is_type_of< world::SpatialEntityData >(*primaryType))
		{
			Ref< world::ExternalSpatialEntityData > spatialEntityData = gc_new< world::ExternalSpatialEntityData >(cref(instance->getGuid()));

			// Initially put entity in front of camera.
			Ref< Camera > camera = m_context->getCamera();
			T_ASSERT (camera);

			Vector4 position = (camera->getTargetView() * translate(0.0f, 0.0f, -4.0f)).inverse().translation();
			spatialEntityData->setTransform(translate(position));

			entityData = spatialEntityData;
		}
		else
			entityData = gc_new< world::ExternalEntityData >(cref(instance->getGuid()));

		m_undoStack->push(m_dataObject);

		// Create instance and adapter.
		Ref< world::EntityInstance > entityInstance = gc_new< world::EntityInstance >(cref(instance->getName()), entityData);
		Ref< EntityAdapter > entityAdapter = gc_new< EntityAdapter >(entityInstance);

		if (parentGroupAdapter)
			parentGroupAdapter->addChild(entityAdapter, true);
		else
			m_context->getSceneAsset()->setInstance(entityInstance);

		updateScene();
		updateInstanceGrid();
		updatePropertyObject();
	}
	else if (is_type_of< world::PostProcessSettings >(*primaryType))
	{
		Ref< world::PostProcessSettings > postProcessSettings = instance->getObject< world::PostProcessSettings >();
		if (postProcessSettings)
			m_editControl->getRenderControl()->setPostProcessSettings(postProcessSettings);
	}
	else
		return false;

	return true;
}

bool SceneEditorPage::handleCommand(const ui::Command& command)
{
	bool result = true;

	if (command == L"Editor.Undo")
	{
		if (!m_undoStack->canUndo())
			return false;

		m_dataObject = m_undoStack->undo(m_dataObject);

		if (Ref< SceneAsset > sceneAsset = dynamic_type_cast< SceneAsset* >(m_dataObject))
		{
			m_context->setSceneAsset(sceneAsset);
		}
		else if (Ref< world::EntityData > entityData = dynamic_type_cast< world::EntityData* >(m_dataObject))
		{
			Ref< SceneAsset > sceneAsset = createWhiteRoomSceneAsset(entityData);
			if (sceneAsset)
				m_context->setSceneAsset(sceneAsset);
			else
				log::warning << L"Unable to undo scene" << Endl;
		}

		updateScene();
		updateInstanceGrid();
		updatePropertyObject();
	}
	else if (command == L"Editor.Redo")
	{
		if (!m_undoStack->canRedo())
			return false;

		m_dataObject = m_undoStack->redo(m_dataObject);

		if (Ref< SceneAsset > sceneAsset = dynamic_type_cast< SceneAsset* >(m_dataObject))
		{
			m_context->setSceneAsset(sceneAsset);
		}
		else if (Ref< world::EntityData > entityData = dynamic_type_cast< world::EntityData* >(m_dataObject))
		{
			Ref< SceneAsset > sceneAsset = createWhiteRoomSceneAsset(entityData);
			if (sceneAsset)
				m_context->setSceneAsset(sceneAsset);
			else
				log::warning << L"Unable to redo scene" << Endl;
		}

		updateScene();
		updateInstanceGrid();
		updatePropertyObject();
	}
	else if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		RefArray< EntityAdapter > selectedEntities;
		if (!m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			return false;

		m_undoStack->push(m_dataObject);

		// Create clipboard data with all selected entities; remove entities from scene if we're cutting.
		Ref< EntityClipboardData > entityClipboardData = gc_new< EntityClipboardData >();
		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
		{
			entityClipboardData->addInstance((*i)->getInstance());
			if (command == L"Editor.Cut")
			{
				Ref< EntityAdapter > parentGroupAdapter = (*i)->getParentContainerGroup();
				if (parentGroupAdapter)
					parentGroupAdapter->removeChild((*i), true);
			}
		}

		ui::Application::getInstance().getClipboard()->setObject(entityClipboardData);

		if (command == L"Editor.Cut")
		{
			updateScene();
			updateInstanceGrid();
		}
	}
	else if (command == L"Editor.Paste")
	{
		// Get parent group under which we will place the new entity.
		RefArray< EntityAdapter > selectedEntities;
		if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
			return false;

		Ref< EntityAdapter > parentEntity = selectedEntities[0]->getParentGroup();
		T_ASSERT (parentEntity);

		// Get clipboard data; ensure correct type.
		Ref< EntityClipboardData > entityClipboardData = dynamic_type_cast< EntityClipboardData* >(
			ui::Application::getInstance().getClipboard()->getObject()
		);
		if (!entityClipboardData)
			return false;

		m_undoStack->push(m_dataObject);

		// Create new instances and adapters for each entity found in clipboard.
		const RefArray< world::EntityInstance >& instances = entityClipboardData->getInstances();
		for (RefArray< world::EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
		{
			Ref< EntityAdapter > adapter = gc_new< EntityAdapter >(*i);
			parentEntity->addChild(adapter, true);
		}

		updateScene();
		updateInstanceGrid();
	}
	else if (command == L"Editor.Delete")
	{
		RefArray< EntityAdapter > selectedEntities;
		if (!m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			return false;

		m_undoStack->push(m_dataObject);

		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
		{
			Ref< EntityAdapter > parentGroupAdapter = (*i)->getParentContainerGroup();
			if (parentGroupAdapter)
				parentGroupAdapter->removeChild((*i), true);
		}

		updateScene();
		updateInstanceGrid();
		updatePropertyObject();
	}
	else if (command == L"Editor.SelectAll")
	{
		m_context->selectAllEntities();

		updateInstanceGrid();
		updatePropertyObject();
	}
	else if (command == L"Scene.Editor.AddEntity")
		result = addEntity();
	else if (command == L"Scene.Editor.MoveToSelectedEntity")
		result = moveToSelectedEntity();
	else if (command == L"Scene.Editor.MoveSelectedEntityIntoView")
		result = moveSelectedEntityIntoView();
	else if (command == L"Scene.Editor.LookAtEntity")
		result = updateCameraLook();
	else
	{
		// Propagate command to editor control.
		result = m_editControl->handleCommand(command);
	}

	return result;
}

void SceneEditorPage::handleDatabaseEvent(const Guid& eventId)
{
	if (m_context)
		m_context->getResourceManager()->update(eventId, true);
}

SceneAsset* SceneEditorPage::createWhiteRoomSceneAsset(world::EntityData* entityData)
{
	// Create temporary instance from entity data.
	Ref< world::EntityInstance > instance = gc_new< world::EntityInstance >(L"Entity", entityData);

	// Read white-room scene asset from system database.
	Ref< SceneAsset > sceneAsset = m_context->getSourceDatabase()->getObjectReadOnly< SceneAsset >(c_guidWhiteRoomScene);
	T_ASSERT_M (sceneAsset, L"Unable to open white-room scene");

	// Add our entity to white-room scene.
	Ref< world::EntityInstance > rootInstance = sceneAsset->getInstance();
	Ref< world::GroupEntityData > rootGroup = checked_type_cast< world::GroupEntityData* >(rootInstance->getEntityData());
	rootGroup->addInstance(instance);

	return sceneAsset;
}

void SceneEditorPage::updateScene()
{
	m_context->buildEntities();

	Ref< world::WorldRenderSettings > newSettings = m_context->getSceneAsset()->getWorldRenderSettings();
	if (newSettings)
	{
		DeepHash hash(newSettings);
		if (hash != m_currentSettingsMD5)
		{
			m_editControl->setWorldRenderSettings(newSettings);
			m_currentSettingsMD5 = hash.getMD5();
		}
	}
}

ui::custom::GridRow* SceneEditorPage::createEntityListRow(EntityAdapter* entityAdapter)
{
	Ref< ui::custom::GridRow > row = gc_new< ui::custom::GridRow >(0);
	row->setData(L"ENTITY", entityAdapter);
	row->setState(entityAdapter->isSelected() ? ui::custom::GridRow::RsSelected : 0);

	// All external entities is highlighted as they shouldn't be edited.
	if (entityAdapter->isChildOfExternal())
	{
		row->addItem(gc_new< ui::custom::GridItem >(entityAdapter->getName(), 0));
		row->setFont(m_instanceGridFontItalic);
	}
	else if (entityAdapter->isExternal())
	{
		row->addItem(gc_new< ui::custom::GridItem >(entityAdapter->getName(), 1));
		row->setFont(m_instanceGridFontBold);
	}
	else if (entityAdapter->isGroup())
		row->addItem(gc_new< ui::custom::GridItem >(entityAdapter->getName(), 2, 3));
	else
		row->addItem(gc_new< ui::custom::GridItem >(entityAdapter->getName(), 0));

	// Recursively add children.
	const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
	for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		Ref< ui::custom::GridRow > child = createEntityListRow(*i);
		if (child)
			row->addChild(child);
	}

	return row;
}

void SceneEditorPage::updateInstanceGrid()
{
	m_instanceGrid->removeAllRows();

	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters);

	for (RefArray< EntityAdapter >::iterator j = entityAdapters.begin(); j != entityAdapters.end(); ++j)
	{
		EntityAdapter* entityAdapter = *j;
		if (entityAdapter->getParent())
			continue;

		Ref< ui::custom::GridRow > entityRow = createEntityListRow(entityAdapter);
		if (entityRow)
		{
			entityRow->setState(entityRow->getState() | ui::custom::GridRow::RsExpanded);
			m_instanceGrid->addRow(entityRow);
		}
	}

	m_instanceGrid->update();
}

void SceneEditorPage::updatePropertyObject()
{
	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters, SceneEditorContext::GfDescendants | SceneEditorContext::GfSelectedOnly);

	if (entityAdapters.size() == 1)
	{
		Ref< EntityAdapter > entityAdapter = entityAdapters.front();
		T_ASSERT (entityAdapter);

		m_context->getEditor()->setPropertyObject(entityAdapter->getEntityData());
	}
	else
	{
		if (is_a< SceneAsset >(m_dataObject))
			m_context->getEditor()->setPropertyObject(static_cast< SceneAsset* >(m_dataObject.getPtr())->getWorldRenderSettings());
		else
			m_context->getEditor()->setPropertyObject(m_dataObject);
	}
}

bool SceneEditorPage::addEntity()
{
	Ref< EntityAdapter > parentGroupAdapter;

	// Get selected entity, must be a single item.
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
	{
		parentGroupAdapter = selectedEntities[0]->getParentGroup();
		if (!parentGroupAdapter)
				return false;
	}
	else if (m_context->getSceneAsset()->getInstance())
	{
		parentGroupAdapter = m_context->getRootEntityAdapter();
		if (!parentGroupAdapter)
				return false;
	}

	// Select type of entity to create.
	const Type* entityType = m_context->getEditor()->browseType(&type_of< world::EntityData >());
	if (!entityType)
		return false;

	Ref< world::EntityData > entityData = checked_type_cast< world::EntityData* >(entityType->newInstance());
	T_ASSERT (entityData);

	m_undoStack->push(m_dataObject);

	// Create instance and adapter.
	Ref< world::EntityInstance > instance = gc_new< world::EntityInstance >(
		i18n::Text(L"SCENE_EDITOR_UNNAMED_ENTITY"),
		entityData
	);
	Ref< EntityAdapter > entityAdapter = gc_new< EntityAdapter >(instance);

	if (parentGroupAdapter)
		parentGroupAdapter->addChild(entityAdapter, true);
	else
		m_context->getSceneAsset()->setInstance(instance);

	updateScene();
	updateInstanceGrid();
	updatePropertyObject();
	return true;
}

bool SceneEditorPage::moveToSelectedEntity()
{
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
		return false;

	if (!selectedEntities[0]->isSpatial())
		return false;

	Aabb boundingBox = selectedEntities[0]->getBoundingBox();
	if (!boundingBox.empty())
	{
		// Calculate target transform so entity is visible in front of camera.
		Matrix44 targetTransform = selectedEntities[0]->getTransform();
		targetTransform = translate(targetTransform.translation());
		targetTransform *= translate(-targetTransform.axisZ() * (boundingBox.getExtent().length() + Scalar(2.0f)));

		// Move camera to new target location.
		Ref< Camera > camera = m_context->getCamera();
		camera->setTargetView(targetTransform);

		// Ensure look-at tool isn't toggled as camera is
		// automatically reset to free mode when directly
		// modifying target view.
		m_toolLookAtEntity->setToggled(false);
		m_entityToolBar->update();
	}
	
	return true;
}

bool SceneEditorPage::moveSelectedEntityIntoView()
{
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
		return false;

	if (!selectedEntities[0]->isSpatial())
		return false;

	m_undoStack->push(m_dataObject);

	Ref< Camera > camera = m_context->getCamera();
	T_ASSERT (camera);

	Aabb boundingBox = selectedEntities[0]->getBoundingBox();
	float distance = boundingBox.getExtent().length() + 2.0f;

	Vector4 position = (camera->getTargetView() * translate(0.0f, 0.0f, -distance)).inverse().translation();

	Matrix44 entityTransform = selectedEntities[0]->getTransform();
	entityTransform(3) = position.xyz1();

	selectedEntities[0]->setTransform(entityTransform);
	return true;
}

bool SceneEditorPage::updateCameraLook()
{
	if (m_toolLookAtEntity->isToggled())
	{
		RefArray< EntityAdapter > selectedEntities;
		if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1 || !selectedEntities[0]->isSpatial())
		{
			m_toolLookAtEntity->setToggled(false);
			m_entityToolBar->update();
			return false;
		}

		m_context->getCamera()->enterLookAt(selectedEntities[0]->getTransform().translation());
	}
	else
	{
		m_context->getCamera()->enterFreeLook();
	}
	return true;
}

void SceneEditorPage::eventEntityToolClick(ui::Event* event)
{
	ui::CommandEvent* commandEvent = checked_type_cast< ui::CommandEvent* >(event);
	handleCommand(commandEvent->getCommand());
}

void SceneEditorPage::eventInstanceSelect(ui::Event* event)
{
	// De-select all entities.
	m_context->selectAllEntities(false);

	// Select only entities which is selected in the grid.
	RefArray< ui::custom::GridRow > selectedRows;
	m_instanceGrid->getRows(selectedRows, ui::custom::GridView::GfDescendants | ui::custom::GridView::GfSelectedOnly);

	for (RefArray< ui::custom::GridRow >::iterator i = selectedRows.begin(); i != selectedRows.end(); ++i)
	{
		EntityAdapter* entityAdapter = (*i)->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT (entityAdapter);

		m_context->selectEntity(entityAdapter);
	}

	updatePropertyObject();
}

void SceneEditorPage::eventInstanceButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() == ui::MouseEvent::BtRight)
	{
		Ref< ui::MenuItem > selectedItem = m_entityMenu->show(m_instanceGrid, mouseEvent->getPosition());
		if (selectedItem)
		{
			if (handleCommand(selectedItem->getCommand()))
				mouseEvent->consume();
		}
	}
}

void SceneEditorPage::eventInstanceDoubleClick(ui::Event* event)
{
	RefArray< ui::custom::GridRow > selectedRows;
	if (m_instanceGrid->getRows(selectedRows, ui::custom::GridView::GfDescendants | ui::custom::GridView::GfSelectedOnly) != 1)
		return;

	Ref< world::EntityInstance > instance = selectedRows[0]->getData< EntityAdapter >(L"ENTITY")->getInstance();

	ui::custom::InputDialog::Field fields[] =
	{
		{ i18n::Text(L"SCENE_EDITOR_NAME"), instance->getName(), 0 }
	};

	ui::custom::InputDialog inputDialog;
	if (!inputDialog.create(m_instanceGrid, i18n::Text(L"SCENE_EDITOR_ENTER_NAME"), i18n::Text(L"SCENE_EDITOR_ENTER_NAME"), fields, sizeof_array(fields)))
		return;

	if (inputDialog.showModal() == ui::DrOk)
		instance->setName(fields[0].value);

	inputDialog.destroy();
	event->consume();
}

void SceneEditorPage::eventContextPostBuild(ui::Event* event)
{
	updateInstanceGrid();
}

void SceneEditorPage::eventContextSelect(ui::Event* event)
{
	updateInstanceGrid();
}

void SceneEditorPage::eventContextPreModify(ui::Event* event)
{
	m_undoStack->push(m_dataObject);
}

	}
}
