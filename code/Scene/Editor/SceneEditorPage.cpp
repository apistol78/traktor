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
#include "Editor/Editor.h"
#include "Editor/Settings.h"
#include "Editor/UndoStack.h"
#include "Database/Database.h"
#include "Physics/PhysicsManager.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/SpatialEntity.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalSpatialEntityData.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "Resource/ResourceManager.h"
#include "Resource/ResourceCache.h"
#include "Resource/ResourceLoader.h"
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
#include "I18N/Text.h"
#include "Database/Instance.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Log/Log.h"

// Resources
#include "Resources/EntityEdit.h"
#include "Resources/EntityTypes.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneEditorPage", SceneEditorPage, editor::EditorPage)

SceneEditorPage::SceneEditorPage(SceneEditorContext* context)
:	m_context(context)
{
}

bool SceneEditorPage::create(ui::Container* parent)
{
	// Create entity editor.
	if (!createEntityEditors())
		return false;

	// Create resource loaders.
	m_resourceCache = gc_new< resource::ResourceCache >();
	m_resourceLoader = gc_new< resource::ResourceLoader >();

	for (RefArray< SceneEditorProfile >::iterator i = m_context->getEditorProfiles().begin(); i != m_context->getEditorProfiles().end(); ++i)
	{
		RefArray< resource::ResourceFactory > resourceFactories;
		(*i)->createResourceFactories(m_context, resourceFactories);

		for (RefArray< resource::ResourceFactory >::iterator j = resourceFactories.begin(); j != resourceFactories.end(); ++j)
			m_resourceLoader->addFactory(*j);
	}

	// Set resource cache and loaders as the preview control needs to load shaders for the primitive renderer.
	resource::ResourceManager::getInstance().setCache(m_resourceCache);
	resource::ResourceManager::getInstance().addLoader(m_resourceLoader);

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

	m_entityGrid = gc_new< ui::custom::GridView >();
	m_entityGrid->create(m_entityPanel, ui::custom::GridView::WsDrag | ui::WsDoubleBuffer);
	m_entityGrid->addImage(ui::Bitmap::load(c_ResourceEntityTypes, sizeof(c_ResourceEntityTypes), L"png"), 4);
	m_entityGrid->addColumn(gc_new< ui::custom::GridColumn >(i18n::Text(L"SCENE_EDITOR_ENTITY_NAME"), 300));
	m_entityGrid->addSelectEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventEntityGridSelect));
	m_entityGrid->addButtonDownEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventEntityGridButtonDown));
	m_entityGrid->addDoubleClickEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventEntityGridDoubleClick));
	m_entityGrid->addDragValidEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventEntityGridDragValid));

	m_entityGridFontItalic = gc_new< ui::Font >(cref(m_entityGrid->getFont()));
	m_entityGridFontItalic->setItalic(true);

	m_entityGridFontBold = gc_new< ui::Font >(cref(m_entityGrid->getFont()));
	m_entityGridFontBold->setBold(true);

	m_context->getEditor()->createAdditionalPanel(m_entityPanel, 250, false);

	// Context event handlers.
	m_context->addChangeEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextChange));
	m_context->addSelectEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextSelect));

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

	// Flush our resource cache.
	m_resourceCache->flush();

	// Destroy physics manager.
	if (m_context->getPhysicsManager())
		m_context->getPhysicsManager()->destroy();
}

void SceneEditorPage::activate()
{
	resource::ResourceManager::getInstance().setCache(m_resourceCache);
	resource::ResourceManager::getInstance().addLoader(m_resourceLoader);

	m_editControl->setVisible(true);
	m_context->getEditor()->showAdditionalPanel(m_entityPanel);
}

void SceneEditorPage::deactivate()
{
	m_context->getEditor()->hideAdditionalPanel(m_entityPanel);
	m_editControl->setVisible(false);

	resource::ResourceManager::getInstance().removeLoader(m_resourceLoader);
	resource::ResourceManager::getInstance().setCache(0);
}

bool SceneEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	Ref< SceneAsset > sceneAsset = dynamic_type_cast< SceneAsset* >(data);

	// Hide edit control as we don't want paint events triggered.
	m_editControl->setVisible(false);

	// Prepare resource requests.
	resource::ResourceManager::getInstance().beginPrepareResources();

	if (sceneAsset)
	{
		m_context->setSceneAsset(sceneAsset);

		updateScene(true);
		createEntityList();
	}
	else
	{
		Ref< world::EntityData > entityData = dynamic_type_cast< world::EntityData* >(data);
		if (!entityData)
		{
			m_editControl->setVisible(true);
			resource::ResourceManager::getInstance().endPrepareResources(true);
			return false;
		}

		// Read white-room scene asset from system domain.
		const Guid c_guidWhiteRoomScene(L"{473467B0-835D-EF45-B308-E3C3C5B0F226}");

		sceneAsset = m_context->getSourceDatabase()->getObjectReadOnly< SceneAsset >(c_guidWhiteRoomScene);
		T_ASSERT_M (sceneAsset, L"Unable to open white-room scene");

		// Add our entity to white-room scene.
		Ref< world::GroupEntityData > sceneRootGroup = checked_type_cast< world::GroupEntityData* >(sceneAsset->getEntityData());
		sceneRootGroup->addEntityData(entityData);

		m_context->setSceneAsset(sceneAsset);
		updateScene(true);

		// Put camera so entity is initially visible.
		Ref< EntityAdapter > entityAdapter = m_context->findEntityFromData(entityData, 0);
		T_ASSERT (entityAdapter);

		Aabb boundingBox = entityAdapter->getBoundingBox();
		float distance = !boundingBox.empty() ? boundingBox.getExtent().z() + 4.0f : 4.0f;
		
		m_context->setCamera(gc_new< Camera >(cref(translate(
			0.0f,
			0.0f,
			-distance
		))));

		m_context->selectEntity(entityAdapter);

		// Rebuild entity list.
		createEntityList();
	}

	// Pre-load all resources; create loader thread.
	Thread* prepareResourcesThread = ThreadManager::getInstance().create(makeFunctor(
		&resource::ResourceManager::getInstance(),
		&resource::ResourceManager::endPrepareResources,
		false
	));
	T_ASSERT (prepareResourcesThread);

	// Create loading dialog.
	ui::custom::BackgroundWorkerDialog loadingDialog;
	loadingDialog.create(m_editPanel, i18n::Text(L"SCENE_EDITOR_LOADING_TITLE"), i18n::Text(L"SCENE_EDITOR_LOADING"));
	loadingDialog.execute(prepareResourcesThread, 0);
	loadingDialog.destroy();

	// Destroy loader thread.
	ThreadManager::getInstance().destroy(prepareResourcesThread);

	m_dataObject = checked_type_cast< Serializable* >(data);
	updatePropertyObject();

	// Show preview control.
	m_editControl->setVisible(true);

	return true;
}

Object* SceneEditorPage::getDataObject()
{
	return m_dataObject;
}

void SceneEditorPage::propertiesChanged()
{
	updateScene(true);
	createEntityList();
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
		if (m_entityGrid->getRows(selectedRows, ui::custom::GridView::GfDescendants | ui::custom::GridView::GfSelectedOnly) == 1)
		{
			Ref< EntityAdapter > selectedEntity = selectedRows[0]->getData< EntityAdapter >(L"ENTITY");
			T_ASSERT (selectedEntity);

			parentGroupAdapter = selectedEntity->getParentGroup();
			if (!parentGroupAdapter)
					return false;
		}
		else if (m_context->getSceneAsset()->getEntityData())
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

		entityData->setName(instance->getName());

		m_undoStack->push(m_dataObject);

		// Create adapter and update scene.
		Ref< EntityAdapter > entityAdapter = gc_new< EntityAdapter >(entityData);

		if (parentGroupAdapter)
			parentGroupAdapter->addChild(entityAdapter, true);
		else
			m_context->getSceneAsset()->setEntityData(entityData);

		updateScene(true);
		createEntityList();
		updatePropertyObject();
	}
	else if (is_type_of< world::PostProcessSettings >(*primaryType))
	{
		Ref< world::PostProcessSettings > postProcessSettings = instance->checkout< world::PostProcessSettings >(db::CfReadOnly);
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

		Ref< SceneAsset > sceneAsset = dynamic_type_cast< SceneAsset* >(m_dataObject);
		if (sceneAsset)
			m_context->setSceneAsset(sceneAsset);

		updateScene(true);
		createEntityList();
		updatePropertyObject();
	}
	else if (command == L"Editor.Redo")
	{
		if (!m_undoStack->canRedo())
			return false;

		m_dataObject = m_undoStack->redo(m_dataObject);

		Ref< SceneAsset > sceneAsset = dynamic_type_cast< SceneAsset* >(m_dataObject);
		if (sceneAsset)
			m_context->setSceneAsset(sceneAsset);

		updateScene(true);
		createEntityList();
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
			entityClipboardData->addEntityData((*i)->getEntityData());
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
			updateScene(true);
			createEntityList();
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

		// Create new adapters for each entity found in clipboard.
		const RefArray< world::EntityData >& entityData = entityClipboardData->getEntityData();
		for (RefArray< world::EntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
			parentEntity->addChild(gc_new< EntityAdapter >(*i), true);

		updateScene(true);
		createEntityList();
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

		updateScene(true);
		createEntityList();
		updatePropertyObject();
	}
	else if (command == L"Editor.SelectAll")
	{
		m_context->selectAllEntities();

		createEntityList();
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
	if (m_resourceCache)
		m_resourceCache->flush(eventId);

	// Check all external entities; might be such an entity which has been modified.
	bool anyExternalDirty = false;
	if (m_context)
	{
		RefArray< EntityAdapter > entityAdapters;
		m_context->getEntities(entityAdapters, SceneEditorContext::GfDescendants);

		for (RefArray< EntityAdapter >::iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
		{
			Guid externalGuid;
			if ((*i)->getExternalGuid(externalGuid) && externalGuid == eventId)
			{
				(*i)->forceModified();
				anyExternalDirty = true;
			}
		}
	}

	if (anyExternalDirty)
	{
		updateScene(false);
		createEntityList();
	}
	else
		updateScene(true);
}

bool SceneEditorPage::createEntityEditors()
{
	RefArray< EntityEditor > entityEditors;
	for (RefArray< SceneEditorProfile >::iterator i = m_context->getEditorProfiles().begin(); i != m_context->getEditorProfiles().end(); ++i)
	{
		RefArray< EntityEditor > profileEntityEditors;
		(*i)->createEntityEditors(m_context, profileEntityEditors);
		entityEditors.insert(entityEditors.end(), profileEntityEditors.begin(), profileEntityEditors.end());
	}
	m_context->setEntityEditors(entityEditors);
	return true;
}

void SceneEditorPage::updateScene(bool updateModified)
{
	if (updateModified)
		m_context->updateModified();

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
		row->setFont(m_entityGridFontItalic);
	}
	else if (entityAdapter->isExternal())
	{
		row->addItem(gc_new< ui::custom::GridItem >(entityAdapter->getName(), 1));
		row->setFont(m_entityGridFontBold);
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

void SceneEditorPage::createEntityList()
{
	m_entityGrid->removeAllRows();

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
			m_entityGrid->addRow(entityRow);
		}
	}

	m_entityGrid->update();
}

void SceneEditorPage::updateEntityList()
{
	RefArray< ui::custom::GridRow > rows;
	m_entityGrid->getRows(rows, ui::custom::GridView::GfDescendants);

	for (RefArray< ui::custom::GridRow >::iterator i = rows.begin(); i != rows.end(); ++i)
	{
		Ref< EntityAdapter > entityAdapter = (*i)->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT (entityAdapter);

		uint32_t state = (*i)->getState();

		if (entityAdapter->isSelected())
		{
			// Expand all parents to ensure new selection is made visible.
			if ((state & ui::custom::GridRow::RsSelected) == 0)
			{
				state |= ui::custom::GridRow::RsSelected;
				for (Ref< ui::custom::GridRow > parent = (*i)->getParent(); parent; parent = parent->getParent())
					parent->setState(parent->getState() | ui::custom::GridRow::RsExpanded);
			}
		}
		else
			state &= ~ui::custom::GridRow::RsSelected;

		(*i)->setState(state);
	}

	m_entityGrid->update();
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
	else if (m_context->getSceneAsset()->getEntityData())
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

	entityData->setName(i18n::Text(L"SCENE_EDITOR_UNNAMED_ENTITY"));

	m_undoStack->push(m_dataObject);

	Ref< EntityAdapter > entityAdapter = gc_new< EntityAdapter >(entityData);

	if (parentGroupAdapter)
		parentGroupAdapter->addChild(entityAdapter, true);
	else
		m_context->getSceneAsset()->setEntityData(entityData);

	updateScene(true);
	createEntityList();
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

void SceneEditorPage::eventEntityGridSelect(ui::Event* event)
{
	// De-select all entities.
	m_context->selectAllEntities(false);

	// Select only entities which is selected in the grid.
	RefArray< ui::custom::GridRow > selectedRows;
	m_entityGrid->getRows(selectedRows, ui::custom::GridView::GfDescendants | ui::custom::GridView::GfSelectedOnly);

	for (RefArray< ui::custom::GridRow >::iterator i = selectedRows.begin(); i != selectedRows.end(); ++i)
	{
		EntityAdapter* entityAdapter = (*i)->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT (entityAdapter);

		m_context->selectEntity(entityAdapter);
	}

	updatePropertyObject();
}

void SceneEditorPage::eventEntityGridButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() == ui::MouseEvent::BtRight)
	{
		Ref< ui::MenuItem > selectedItem = m_entityMenu->show(m_entityGrid, mouseEvent->getPosition());
		if (selectedItem)
		{
			if (handleCommand(selectedItem->getCommand()))
				mouseEvent->consume();
		}
	}
}

void SceneEditorPage::eventEntityGridDoubleClick(ui::Event* event)
{
	ui::Command command(L"Scene.Editor.MoveToSelectedEntity");
	handleCommand(command);
}

void SceneEditorPage::eventEntityGridDragValid(ui::Event* event)
{
	Ref< ui::custom::GridDragEvent > dragEvent = checked_type_cast< ui::custom::GridDragEvent* >(event);
	Ref< ui::custom::GridRow > row = checked_type_cast< ui::custom::GridRow* >(dragEvent->getItem());

	Ref< EntityAdapter > entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
	T_ASSERT (entityAdapter);

	// Cannot drag root entities.
	if (!entityAdapter->getParent())
		dragEvent->cancel();

	dragEvent->consume();
}

void SceneEditorPage::eventContextChange(ui::Event* event)
{
	createEntityList();
}

void SceneEditorPage::eventContextSelect(ui::Event* event)
{
	updateEntityList();
}

	}
}
