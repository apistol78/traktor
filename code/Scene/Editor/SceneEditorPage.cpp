#include "Core/Log/Log.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Settings/Settings.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/UndoStack.h"
#include "I18N/Text.h"
#include "Physics/PhysicsManager.h"
#include "Scene/ISceneController.h"
#include "Scene/ISceneControllerData.h"
#include "Scene/Scene.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityClipboardData.h"
#include "Scene/Editor/EntityDependencyInvestigator.h"
#include "Scene/Editor/ISceneControllerEditorFactory.h"
#include "Scene/Editor/ISceneControllerEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/SceneEditorPage.h"
#include "Scene/Editor/ScenePreviewControl.h"
#include "Scene/Editor/SelectEvent.h"
#include "Resource/IResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/PopupMenu.h"
#include "Ui/TableLayout.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/InputDialog.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/SpatialEntity.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalSpatialEntityData.h"

// Resources
#include "Resources/EntityEdit.h"
#include "Resources/EntityTypes.h"
#include "Resources/LayerHidden.h"
#include "Resources/LayerVisible.h"
#include "Resources/LayerLocked.h"
#include "Resources/LayerUnlocked.h"

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

bool SceneEditorPage::create(ui::Container* parent, editor::IEditorPageSite* site)
{
	m_site = site;
	T_ASSERT (m_site);

	// Create editor panel.
	m_editPanel = new ui::Container();
	m_editPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));

	m_editControl = new ScenePreviewControl();
	m_editControl->create(m_editPanel, m_context);

	// Create entity panel.
	m_entityPanel = new ui::Container();
	m_entityPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));
	m_entityPanel->setText(i18n::Text(L"SCENE_EDITOR_ENTITIES"));

	m_entityMenu = new ui::PopupMenu();
	m_entityMenu->create();
	m_entityMenu->add(new ui::MenuItem(ui::Command(L"Scene.Editor.AddEntity"), i18n::Text(L"SCENE_EDITOR_ADD_ENTITY")));
	m_entityMenu->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY")));

	m_toolLookAtEntity = new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_LOOK_AT_ENTITY"), ui::Command(L"Scene.Editor.LookAtEntity"), 3, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolFollowEntity = new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_FOLLOW_ENTITY"), ui::Command(L"Scene.Editor.FollowEntity"), 4, ui::custom::ToolBarButton::BsDefaultToggle);

	m_entityToolBar = new ui::custom::ToolBar();
	m_entityToolBar->create(m_entityPanel);
	m_entityToolBar->addImage(ui::Bitmap::load(c_ResourceEntityEdit, sizeof(c_ResourceEntityEdit), L"png"), 5);
	m_entityToolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY"), ui::Command(L"Editor.Delete"), 2));
	m_entityToolBar->addItem(new ui::custom::ToolBarSeparator());
	m_entityToolBar->addItem(m_toolLookAtEntity);
	m_entityToolBar->addItem(m_toolFollowEntity);
	m_entityToolBar->addClickEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventEntityToolClick));

	m_imageHidden = ui::Bitmap::load(c_ResourceLayerHidden, sizeof(c_ResourceLayerHidden), L"png");
	m_imageVisible = ui::Bitmap::load(c_ResourceLayerVisible, sizeof(c_ResourceLayerVisible), L"png");
	m_imageLocked = ui::Bitmap::load(c_ResourceLayerLocked, sizeof(c_ResourceLayerLocked), L"png");
	m_imageUnlocked = ui::Bitmap::load(c_ResourceLayerUnlocked, sizeof(c_ResourceLayerUnlocked), L"png");

	m_instanceGrid = new ui::custom::GridView();
	m_instanceGrid->create(m_entityPanel, ui::WsDoubleBuffer);
	//m_instanceGrid->addImage(ui::Bitmap::load(c_ResourceEntityTypes, sizeof(c_ResourceEntityTypes), L"png"), 4);
	m_instanceGrid->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCENE_EDITOR_ENTITY_NAME"), 200));
	m_instanceGrid->addColumn(new ui::custom::GridColumn(L"", 30));
	m_instanceGrid->addColumn(new ui::custom::GridColumn(L"", 30));
	m_instanceGrid->addSelectEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventInstanceSelect));
	m_instanceGrid->addExpandEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventInstanceExpand));
	m_instanceGrid->addButtonDownEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventInstanceButtonDown));
	m_instanceGrid->addClickEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventInstanceClick));

	m_instanceGridFontItalic = new ui::Font(m_instanceGrid->getFont());
	m_instanceGridFontItalic->setItalic(true);

	m_instanceGridFontBold = new ui::Font(m_instanceGrid->getFont());
	m_instanceGridFontBold->setBold(true);

	m_site->createAdditionalPanel(m_entityPanel, 300, false);

	// Create dependency panel.
	m_entityDependencyPanel = new EntityDependencyInvestigator(m_context);
	m_entityDependencyPanel->create(parent);

	m_site->createAdditionalPanel(m_entityDependencyPanel, 300, false);

	// Create controller panel.
	m_controllerPanel = new ui::Container();
	m_controllerPanel->create(parent, ui::WsNone, new ui::FloodLayout());
	m_controllerPanel->setText(i18n::Text(L"SCENE_EDITOR_CONTROLLER"));

	m_site->createAdditionalPanel(m_controllerPanel, 120, true);

	// Context event handlers.
	m_context->addPostBuildEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextPostBuild));
	m_context->addSelectEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextSelect));
	m_context->addPreModifyEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextPreModify));
	m_context->addPostModifyEventHandler(ui::createMethodHandler(this, &SceneEditorPage::eventContextPostModify));

	// Restore camera from settings.
	Ref< Settings > settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	m_undoStack = new editor::UndoStack();

	return true;
}

void SceneEditorPage::destroy()
{
	// Save camera position in editor settings.
	Ref< Settings > settings = m_context->getEditor()->getSettings();
	T_ASSERT (settings);

	// Destroy controller editor.
	if (m_context->getControllerEditor())
		m_context->getControllerEditor()->destroy();

	// Destroy panels.
	m_site->destroyAdditionalPanel(m_entityPanel);
	m_site->destroyAdditionalPanel(m_entityDependencyPanel);
	m_site->destroyAdditionalPanel(m_controllerPanel);

	// Destroy widgets.
	safeDestroy(m_entityMenu);
	safeDestroy(m_entityPanel);
	safeDestroy(m_entityDependencyPanel);
	safeDestroy(m_controllerPanel);
	safeDestroy(m_editPanel);

	// Destroy physics manager.
	if (m_context->getPhysicsManager())
		m_context->getPhysicsManager()->destroy();
}

void SceneEditorPage::activate()
{
	m_editControl->setVisible(true);
}

void SceneEditorPage::deactivate()
{
	m_editControl->setVisible(false);
}

bool SceneEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	EnterLeave scopeVisible(
		makeFunctor< ScenePreviewControl, bool >(m_editControl, &ScenePreviewControl::setVisible, false),
		makeFunctor< ScenePreviewControl, bool >(m_editControl, &ScenePreviewControl::setVisible, true)
	);

	m_dataInstance = instance;

	m_context->setInstance(instance);

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

		updateInstanceGrid();
	}

	createControllerEditor();

	m_dataObject = checked_type_cast< ISerializable* >(data);
	updatePropertyObject();

	return true;
}

Ref< db::Instance > SceneEditorPage::getDataInstance()
{
	return m_dataInstance;
}

Ref< Object > SceneEditorPage::getDataObject()
{
	return m_dataObject;
}

bool SceneEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	const TypeInfo* primaryType = instance->getPrimaryType();
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
		else if (m_context->getSceneAsset()->getEntityData())
		{
			parentGroupAdapter = m_context->getRootEntityAdapter();
			if (!parentGroupAdapter)
					return false;
		}

		Ref< world::EntityData > entityData;

		// Create external reference to entity data.
		if (is_type_of< world::SpatialEntityData >(*primaryType))
			entityData = new world::ExternalSpatialEntityData(instance->getGuid());
		else
			entityData = new world::ExternalEntityData(instance->getGuid());

		// Use name of instance as external entity's name.
		entityData->setName(instance->getName());

		m_undoStack->push(m_dataObject);

		// Create instance and adapter.
		Ref< EntityAdapter > entityAdapter = new EntityAdapter(entityData);

		if (parentGroupAdapter)
			parentGroupAdapter->addChild(entityAdapter);
		else
			m_context->getSceneAsset()->setEntityData(entityData);

		updateScene();
		updateInstanceGrid();
		updatePropertyObject();
	}
		return false;

	return true;
}

bool SceneEditorPage::handleCommand(const ui::Command& command)
{
	bool result = true;

	if (command == L"Editor.PropertiesChanged")
	{
		updateScene();
		updateInstanceGrid();

		// Notify controller editor as well.
		Ref< ISceneControllerEditor > controllerEditor = m_context->getControllerEditor();
		if (controllerEditor)
			controllerEditor->propertiesChanged();
	}
	else if (command == L"Editor.Undo")
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
		Ref< EntityClipboardData > entityClipboardData = new EntityClipboardData();
		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
		{
			entityClipboardData->addEntityData((*i)->getEntityData());
			if (command == L"Editor.Cut")
			{
				Ref< EntityAdapter > parentGroup = (*i)->getParent();
				if (parentGroup->isGroup())
				{
					parentGroup->removeChild(*i);

					if (m_context->getControllerEditor())
						m_context->getControllerEditor()->entityRemoved(*i);
				}
			}
		}

		ui::Application::getInstance()->getClipboard()->setObject(entityClipboardData);

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
			ui::Application::getInstance()->getClipboard()->getObject()
		);
		if (!entityClipboardData)
			return false;

		m_undoStack->push(m_dataObject);

		// Create new instances and adapters for each entity found in clipboard.
		const RefArray< world::EntityData >& entityData = entityClipboardData->getEntityData();
		for (RefArray< world::EntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
		{
			Ref< EntityAdapter > adapter = new EntityAdapter(*i);
			parentEntity->addChild(adapter);
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

		uint32_t removedCount = 0;
		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
		{
			Ref< EntityAdapter > parentGroup = (*i)->getParent();
			if (parentGroup && parentGroup->isGroup())
			{
				parentGroup->removeChild(*i);
				removedCount++;

				if (m_context->getControllerEditor())
					m_context->getControllerEditor()->entityRemoved(*i);
			}
		}

		if (removedCount)
		{
			updateScene();
			updateInstanceGrid();
			updatePropertyObject();
		}
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
		result = updateLookAtEntity();
	else if (command == L"Scene.Editor.FollowEntity")
		result = updateFollowEntity();
	else
	{
		result = false;

		// Propagate command to controller editor.
		if (!result && m_context->getControllerEditor())
			result = m_context->getControllerEditor()->handleCommand(command);

		// Propagate command to editor control.
		if (!result)
			result = m_editControl->handleCommand(command);
	}

	return result;
}

void SceneEditorPage::handleDatabaseEvent(const Guid& eventId)
{
	if (!m_context)
		return;

	m_context->getResourceManager()->update(eventId, true);

	RefArray< EntityAdapter > entityAdapters;
	m_context->getEntities(entityAdapters);

	bool externalModified = false;
	for (RefArray< EntityAdapter >::const_iterator i = entityAdapters.begin(); i != entityAdapters.end(); ++i)
	{
		Guid externalGuid;
		if ((*i)->getExternalGuid(externalGuid))
		{
			if (externalGuid == eventId)
			{
				// Modified external entity detected; need to recreate the scene.
				externalModified = true;
				break;
			}
		}
	}

	if (externalModified)
	{
		updateScene();
		updateInstanceGrid();
	}
}

void SceneEditorPage::createControllerEditor()
{
	if (m_context->getControllerEditor())
	{
		m_context->getControllerEditor()->destroy();
		m_context->setControllerEditor(0);
	}

	Ref< SceneAsset > sceneAsset = m_context->getSceneAsset();
	if (sceneAsset)
	{
		Ref< ISceneControllerData > controllerData = sceneAsset->getControllerData();
		if (controllerData)
		{
			RefArray< ISceneControllerEditorFactory > controllerEditorFactories;
			Ref< ISceneControllerEditor > controllerEditor;

			// Create controller editor factories.
			const RefArray< ISceneEditorProfile >& profiles = m_context->getEditorProfiles();
			for (RefArray< ISceneEditorProfile >::const_iterator i = profiles.begin(); i != profiles.end(); ++i)
				(*i)->createControllerEditorFactories(m_context, controllerEditorFactories);

			for (RefArray< ISceneControllerEditorFactory >::iterator i = controllerEditorFactories.begin(); i != controllerEditorFactories.end(); ++i)
			{
				TypeInfoSet typeSet = (*i)->getControllerDataTypes();
				if (typeSet.find(&type_of(controllerData)) != typeSet.end())
				{
					controllerEditor = (*i)->createControllerEditor(type_of(controllerData));
					if (controllerEditor)
						break;
				}
			}

			if (controllerEditor)
			{
				if (controllerEditor->create(
					m_context,
					m_controllerPanel
				))
				{
					m_context->setControllerEditor(controllerEditor);
					m_controllerPanel->update();
				}
				else
					log::error << L"Unable to create controller editor; create failed" << Endl;
			}
			else
				log::debug << L"Unable to find controller editor for type \"" << type_name(controllerData) << L"\"" << Endl;
		}
	}
}

Ref< SceneAsset > SceneEditorPage::createWhiteRoomSceneAsset(world::EntityData* entityData)
{
	// Read white-room scene asset from system database.
	Ref< SceneAsset > sceneAsset = m_context->getSourceDatabase()->getObjectReadOnly< SceneAsset >(c_guidWhiteRoomScene);
	T_ASSERT_M (sceneAsset, L"Unable to open white-room scene");

	// Add our entity to white-room scene.
	Ref< world::GroupEntityData > rootGroup = checked_type_cast< world::GroupEntityData*, false >(sceneAsset->getEntityData());
	rootGroup->addEntityData(entityData);

	return sceneAsset;
}

void SceneEditorPage::updateScene()
{
	m_context->buildEntities();

	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	if (sceneAsset)
	{
		// Check if any scene settings has changed.
		bool needUpdate = false;

		if (m_currentGuid != sceneAsset->getPostProcessSettings().getGuid())
		{
			needUpdate = true;
			m_currentGuid = sceneAsset->getPostProcessSettings().getGuid();
		}

		DeepHash hash(sceneAsset->getWorldRenderSettings());
		if (hash != m_currentHash)
		{
			needUpdate = true;
			m_currentHash = hash.get();
		}

		// Inform editor controls to update their world renderer.
		if (needUpdate)
			m_editControl->updateWorldRenderer();
	}
}

Ref< ui::custom::GridRow > SceneEditorPage::createEntityListRow(EntityAdapter* entityAdapter)
{
	Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);
	row->setData(L"ENTITY", entityAdapter);
	row->setState(
		(entityAdapter->isSelected() ? ui::custom::GridRow::RsSelected : 0) |
		(entityAdapter->isExpanded() ? ui::custom::GridRow::RsExpanded : 0)
	);
	
	std::wstring entityName = entityAdapter->getName();
	if (entityName.empty())
		entityName = i18n::Text(L"SCENE_EDITOR_UNNAMED_ENTITY");

	// All external entities is highlighted as they shouldn't be edited.
	if (entityAdapter->isChildOfExternal())
	{
		row->add(new ui::custom::GridItem(entityName/*, 0*/));
		row->setFont(m_instanceGridFontItalic);
	}
	else if (entityAdapter->isExternal())
	{
		row->add(new ui::custom::GridItem(entityName/*, 1*/));
		row->setFont(m_instanceGridFontBold);
	}
	else if (entityAdapter->isGroup())
		row->add(new ui::custom::GridItem(entityName/*, 2, 3*/));
	else
		row->add(new ui::custom::GridItem(entityName/*, 0*/));

	// Create "visible" check box.
	row->add(new ui::custom::GridItem(
		entityAdapter->isVisible(false) ? m_imageVisible : m_imageHidden
	));

	// Create "locked" check box.
	row->add(new ui::custom::GridItem(
		entityAdapter->isLocked(false) ? m_imageLocked : m_imageUnlocked
	));

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
			entityRow->setState(entityRow->getState());
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

		m_site->setPropertyObject(entityAdapter->getEntityData());
	}
	else
		m_site->setPropertyObject(m_dataObject);
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
	const TypeInfo* entityType = m_context->getEditor()->browseType(&type_of< world::EntityData >());
	if (!entityType)
		return false;

	Ref< world::EntityData > entityData = checked_type_cast< world::EntityData* >(entityType->createInstance());
	T_ASSERT (entityData);

	m_undoStack->push(m_dataObject);

	if (parentGroupAdapter)
	{
		Ref< EntityAdapter > entityAdapter = new EntityAdapter(entityData);
		parentGroupAdapter->addChild(entityAdapter);
	}
	else
		m_context->getSceneAsset()->setEntityData(entityData);

	updateScene();
	updateInstanceGrid();
	updatePropertyObject();
	return true;
}

bool SceneEditorPage::moveToSelectedEntity()
{
	//RefArray< EntityAdapter > selectedEntities;
	//if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
	//	return false;

	//if (!selectedEntities[0]->isSpatial())
	//	return false;

	//Aabb boundingBox = selectedEntities[0]->getBoundingBox();
	//if (!boundingBox.empty())
	//{
	//	// Calculate target transform so entity is visible in front of camera.
	//	Matrix44 targetTransform = selectedEntities[0]->getTransform();
	//	targetTransform = translate(targetTransform.translation());
	//	targetTransform *= translate(-targetTransform.axisZ() * (boundingBox.getExtent().length() + Scalar(2.0f)));

	//	//// Move camera to new target location.
	//	//Ref< Camera > camera = m_context->getCamera();
	//	//camera->setTargetView(targetTransform);

	//	// Ensure look-at tool isn't toggled as camera is
	//	// automatically reset to free mode when directly
	//	// modifying target view.
	//	m_toolLookAtEntity->setToggled(false);
	//	m_entityToolBar->update();
	//}
	
	return true;
}

bool SceneEditorPage::moveSelectedEntityIntoView()
{
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
		return false;

	if (!selectedEntities[0]->isSpatial())
		return false;

	//m_undoStack->push(m_dataObject);

	//Ref< Camera > camera = m_context->getCamera();
	//T_ASSERT (camera);

	//Aabb boundingBox = selectedEntities[0]->getBoundingBox();
	//float distance = boundingBox.getExtent().length() + 2.0f;

	//Vector4 position = (camera->getTargetView() * translate(0.0f, 0.0f, -distance)).inverse().translation();

	//Matrix44 entityTransform = selectedEntities[0]->getTransform();
	//entityTransform(3) = position.xyz1();

	//selectedEntities[0]->setTransform(entityTransform);
	return true;
}

bool SceneEditorPage::updateLookAtEntity()
{
	if (m_toolLookAtEntity->isToggled())
	{
		RefArray< EntityAdapter > selectedEntities;
		if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1 && selectedEntities[0]->isSpatial())
		{
			Vector4 lookAtPosition = selectedEntities[0]->getTransform().translation().xyz1();
			for (int32_t i = 0; i < 4; ++i)
				m_context->getCamera(i)->enterLookAt(lookAtPosition);
			return true;
		}
	}
	for (int32_t i = 0; i < 4; ++i)
		m_context->getCamera(i)->enterFreeLook();
	return true;
}

bool SceneEditorPage::updateFollowEntity()
{
	if (m_toolFollowEntity->isToggled())
	{
		RefArray< EntityAdapter > selectedEntities;
		if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1 && selectedEntities[0]->isSpatial())
		{
			m_context->setFollowEntityAdapter(selectedEntities[0]);
			return true;
		}
	}
	m_context->setFollowEntityAdapter(0);
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

	// Raise context select event.
	m_context->raiseSelect(this);
}

void SceneEditorPage::eventInstanceExpand(ui::Event* event)
{
	ui::custom::GridRow* row = checked_type_cast< ui::custom::GridRow*, false >(event->getItem());
	EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
	entityAdapter->setExpanded((row->getState() & ui::custom::GridRow::RsExpanded) != 0);
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

void SceneEditorPage::eventInstanceClick(ui::Event* event)
{
	const ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent* >(event);
	const ui::Command& cmd = cmdEvent->getCommand();

	if (cmd.getId() == 1)
	{
		ui::custom::GridRow* row = checked_type_cast< ui::custom::GridRow*, false >(cmdEvent->getItem());
		ui::custom::GridItem* item = checked_type_cast< ui::custom::GridItem*, false >(row->get(1));

		EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT (entityAdapter);

		if (entityAdapter->isVisible(false))
		{
			item->setImage(m_imageHidden);
			entityAdapter->setVisible(false);
		}
		else
		{
			item->setImage(m_imageVisible);
			entityAdapter->setVisible(true);
		}

		m_instanceGrid->update();
	}
	else if (cmd.getId() == 2)
	{
		ui::custom::GridRow* row = checked_type_cast< ui::custom::GridRow*, false >(cmdEvent->getItem());
		ui::custom::GridItem* item = checked_type_cast< ui::custom::GridItem*, false >(row->get(2));

		EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT (entityAdapter);

		if (entityAdapter->isLocked())
		{
			item->setImage(m_imageUnlocked);
			entityAdapter->setLocked(false);
		}
		else
		{
			item->setImage(m_imageLocked);
			entityAdapter->setLocked(true);
		}

		m_instanceGrid->update();
	}
}

void SceneEditorPage::eventContextPostBuild(ui::Event* event)
{
	updateInstanceGrid();
}

void SceneEditorPage::eventContextSelect(ui::Event* event)
{
	// Prevent updating instance grid if we're the one issued the selection event.
	if (event->getItem() != this)
		updateInstanceGrid();

	updatePropertyObject();
}

void SceneEditorPage::eventContextPreModify(ui::Event* event)
{
	m_undoStack->push(m_dataObject);
}

void SceneEditorPage::eventContextPostModify(ui::Event* event)
{
	updatePropertyObject();
}

	}
}
