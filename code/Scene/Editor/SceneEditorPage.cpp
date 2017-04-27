/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "I18N/Text.h"
#include "Physics/PhysicsManager.h"
#include "Render/IRenderSystem.h"
#include "Resource/ResourceManager.h"
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
#include "Scene/Editor/Events/CameraMovedEvent.h"
#include "Scene/Editor/Events/PostBuildEvent.h"
#include "Scene/Editor/Events/PostModifyEvent.h"
#include "Scene/Editor/Events/PreModifyEvent.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/MenuItem.h"
#include "Ui/PopupMenu.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/InputDialog.h"
#include "Ui/Custom/StatusBar/StatusBar.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridColumnClickEvent.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridRowStateChangeEvent.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/EntityEventManager.h"
#include "World/IEntityComponent.h"
#include "World/WorldRenderSettings.h"
#include "World/Editor/LayerEntityData.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const Guid c_guidWhiteRoomScene(L"{473467B0-835D-EF45-B308-E3C3C5B0F226}");

bool isChildEntitySelected(const EntityAdapter* entityAdapter)
{
	const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
	for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		if ((*i)->isSelected())
			return true;

		if (isChildEntitySelected(*i))
			return true;
	}
	return false;
}

bool filterIncludeEntity(const TypeInfo& entityOrComponentType, EntityAdapter* entityAdapter)
{
	if (!entityAdapter->getEntity())
		return true;

	if (is_type_of(entityOrComponentType, type_of(entityAdapter->getEntity())))
		return true;

	if (entityAdapter->getComponent(entityOrComponentType) != 0)
		return true;

	const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
	for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		if (filterIncludeEntity(entityOrComponentType, *i))
			return true;
	}

	return false;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneEditorPage", SceneEditorPage, editor::IEditorPage)

SceneEditorPage::SceneEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
,	m_entityFilterType(0)
{
}

bool SceneEditorPage::create(ui::Container* parent)
{
	std::set< std::wstring > guideIds;

	// Get render system from store.
	render::IRenderSystem* renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	// Create world event manager.
	Ref< world::EntityEventManager > eventManager = new world::EntityEventManager(64);

	// Get physics manager type.
	std::wstring physicsManagerTypeName = m_editor->getSettings()->getProperty< std::wstring >(L"SceneEditor.PhysicsManager");
	const TypeInfo* physicsManagerType = TypeInfo::find(physicsManagerTypeName);
	if (!physicsManagerType)
	{
		log::error << L"Unable to create scene editor; no such physics manager type \"" << physicsManagerTypeName << L"\"." << Endl;
		return false;
	}

	// Create physics manager.
	physics::PhysicsCreateDesc pcd;
	pcd.timeScale = 1.0f;
	pcd.solverIterations = 10;

	Ref< physics::PhysicsManager > physicsManager = checked_type_cast< physics::PhysicsManager* >(physicsManagerType->createInstance());
	if (!physicsManager->create(pcd))
	{
		log::error << L"Unable to create scene editor; failed to create physics manager." << Endl;
		return false;
	}

	// Configure physics manager.
	physicsManager->setGravity(Vector4(0.0f, -9.81f, 0.0f, 0.0f));

	// Create resource manager.
	Ref< resource::IResourceManager > resourceManager = new resource::ResourceManager(m_editor->getOutputDatabase(), true);

	// Create editor context.
	m_context = new SceneEditorContext(
		m_editor,
		m_document,
		m_editor->getOutputDatabase(),
		m_editor->getSourceDatabase(),
		eventManager,
		resourceManager,
		renderSystem,
		physicsManager
	);

	// Create profiles, plugins, resource factories, entity editors and guide ids.
	TypeInfoSet profileTypes;
	type_of< ISceneEditorProfile >().findAllOf(profileTypes);
	for (TypeInfoSet::const_iterator i = profileTypes.begin(); i != profileTypes.end(); ++i)
	{
		Ref< ISceneEditorProfile > profile = dynamic_type_cast< ISceneEditorProfile* >((*i)->createInstance());
		if (!profile)
			continue;

		m_context->addEditorProfile(profile);

		RefArray< ISceneEditorPlugin > editorPlugins;
		profile->createEditorPlugins(m_context, editorPlugins);
		for (RefArray< ISceneEditorPlugin >::iterator j = editorPlugins.begin(); j != editorPlugins.end(); ++j)
			m_context->addEditorPlugin(*j);

		RefArray< const resource::IResourceFactory > resourceFactories;
		profile->createResourceFactories(m_context, resourceFactories);
		for (RefArray< const resource::IResourceFactory >::iterator j = resourceFactories.begin(); j != resourceFactories.end(); ++j)
			resourceManager->addFactory(*j);

		profile->getGuideDrawIds(guideIds);
	}
	m_context->createFactories();

	// Create editor panel.
	m_editPanel = new ui::Container();
	m_editPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, 0));

	m_editControl = new ScenePreviewControl();
	if (!m_editControl->create(m_editPanel, m_context))
	{
		log::error << L"Unable to create scene editor; failed to scene preview." << Endl;
		return false;
	}

	m_statusBar = new ui::custom::StatusBar();
	m_statusBar->create(m_editPanel);

	// Create entity panel.
	m_entityPanel = new ui::Container();
	m_entityPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));
	m_entityPanel->setText(i18n::Text(L"SCENE_EDITOR_ENTITIES"));

	m_entityMenu = new ui::PopupMenu();
	m_entityMenu->create();
	m_entityMenu->add(new ui::MenuItem(ui::Command(L"Scene.Editor.MoveToEntity"), i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY")));
	m_entityMenu->add(new ui::MenuItem(L"-"));
	m_entityMenu->add(new ui::MenuItem(ui::Command(L"Scene.Editor.AddEntity"), i18n::Text(L"SCENE_EDITOR_ADD_ENTITY")));
	m_entityMenu->add(new ui::MenuItem(ui::Command(L"Scene.Editor.AddGroupEntity"), i18n::Text(L"SCENE_EDITOR_ADD_GROUP_ENTITY")));
	m_entityMenu->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY")));

	m_entityMenuExternal = new ui::PopupMenu();
	m_entityMenuExternal->create();
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Scene.Editor.MoveToEntity"), i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY")));
	m_entityMenuExternal->add(new ui::MenuItem(L"-"));
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Scene.Editor.AddEntity"), i18n::Text(L"SCENE_EDITOR_ADD_ENTITY")));
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Scene.Editor.AddGroupEntity"), i18n::Text(L"SCENE_EDITOR_ADD_GROUP_ENTITY")));
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY")));
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Scene.Editor.FindInDatabase"), i18n::Text(L"SCENE_EDITOR_FIND_IN_DATABASE")));

	m_entityToolBar = new ui::custom::ToolBar();
	m_entityToolBar->create(m_entityPanel);
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.RemoveEntity"), 1);
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.MoveToEntity"), 1);
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.FilterEntity"), 1);
	m_entityToolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY"), 0, ui::Command(L"Editor.Delete")));
	m_entityToolBar->addItem(new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY"), 1, ui::Command(L"Scene.Editor.MoveToEntity")));
	
	m_buttonFilterEntity = new ui::custom::ToolBarButton(i18n::Text(L"SCENE_EDITOR_FILTER_ENTITY"), 2, ui::Command(L"Scene.Editor.FilterEntity"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_entityToolBar->addItem(m_buttonFilterEntity);

	m_entityToolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &SceneEditorPage::eventEntityToolClick);

	m_imageHidden = new ui::StyleBitmap(L"Scene.LayerHidden");
	m_imageVisible = new ui::StyleBitmap(L"Scene.LayerVisible");
	m_imageLocked = new ui::StyleBitmap(L"Scene.LayerLocked");
	m_imageUnlocked = new ui::StyleBitmap(L"Scene.LayerUnlocked");

	m_instanceGrid = new ui::custom::GridView();
	m_instanceGrid->create(m_entityPanel, ui::WsDoubleBuffer);
	//m_instanceGrid->addImage(ui::Bitmap::load(c_ResourceEntityTypes, sizeof(c_ResourceEntityTypes), L"png"), 4);
	m_instanceGrid->addColumn(new ui::custom::GridColumn(L"", ui::scaleBySystemDPI(200)));
	m_instanceGrid->addColumn(new ui::custom::GridColumn(L"", ui::scaleBySystemDPI(30)));
	m_instanceGrid->addColumn(new ui::custom::GridColumn(L"", ui::scaleBySystemDPI(30)));
	m_instanceGrid->addEventHandler< ui::SelectionChangeEvent >(this, &SceneEditorPage::eventInstanceSelect);
	m_instanceGrid->addEventHandler< ui::custom::GridRowStateChangeEvent >(this, &SceneEditorPage::eventInstanceExpand);
	m_instanceGrid->addEventHandler< ui::MouseButtonDownEvent >(this, &SceneEditorPage::eventInstanceButtonDown);
	m_instanceGrid->addEventHandler< ui::custom::GridColumnClickEvent >(this, &SceneEditorPage::eventInstanceClick);

	m_instanceGridFontBold = new ui::Font(m_instanceGrid->getFont());
	m_instanceGridFontBold->setBold(true);

	m_instanceGridFontHuge = new ui::Font(m_instanceGrid->getFont());
	m_instanceGridFontHuge->setSize(12);

	m_site->createAdditionalPanel(m_entityPanel, ui::scaleBySystemDPI(300), false);

	m_tabMisc = new ui::Tab();
	m_tabMisc->create(parent, ui::Tab::WsLine);
	m_tabMisc->setText(i18n::Text(L"SCENE_EDITOR_MISC"));

	// Create dependency panel.
	Ref< ui::TabPage > tabPageDependencies = new ui::TabPage();
	tabPageDependencies->create(m_tabMisc, i18n::Text(L"SCENE_EDITOR_DEPENDENCY_INVESTIGATOR"), new ui::FloodLayout());

	m_entityDependencyPanel = new EntityDependencyInvestigator(m_context);
	m_entityDependencyPanel->create(tabPageDependencies);

	// Create guide visibility panel.
	Ref< ui::TabPage > tabPageGuides = new ui::TabPage();
	tabPageGuides->create(m_tabMisc, i18n::Text(L"SCENE_EDITOR_GUIDES"), new ui::FloodLayout());

	m_gridGuides = new ui::custom::GridView();
	m_gridGuides->create(tabPageGuides, ui::WsDoubleBuffer | ui::WsTabStop);
	m_gridGuides->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCENE_EDITOR_GUIDES_NAME"), ui::scaleBySystemDPI(150)));
	m_gridGuides->addColumn(new ui::custom::GridColumn(i18n::Text(L"SCENE_EDITOR_GUIDES_VISIBLE"), ui::scaleBySystemDPI(50)));

	for (std::set< std::wstring >::const_iterator i = guideIds.begin(); i != guideIds.end(); ++i)
	{
		bool shouldDraw = m_editor->getSettings()->getProperty< bool >(L"SceneEditor.Guides/" + *i, true);
		m_context->setDrawGuide(*i, shouldDraw);

		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(*i));
		row->add(new ui::custom::GridItem(shouldDraw ? m_imageVisible : m_imageHidden));
		m_gridGuides->addRow(row);
	}

	m_gridGuides->addEventHandler< ui::custom::GridColumnClickEvent >(this, &SceneEditorPage::eventGuideClick);

	// Add pages.
	m_tabMisc->addPage(tabPageDependencies);
	m_tabMisc->addPage(tabPageGuides);
	m_tabMisc->setActivePage(tabPageDependencies);

	m_site->createAdditionalPanel(m_tabMisc, ui::scaleBySystemDPI(300), false);

	// Create controller panel.
	m_controllerPanel = new ui::Container();
	m_controllerPanel->create(parent, ui::WsNone, new ui::FloodLayout());
	m_controllerPanel->setText(i18n::Text(L"SCENE_EDITOR_CONTROLLER"));

	m_site->createAdditionalPanel(m_controllerPanel, ui::scaleBySystemDPI(120), true);

	// Context event handlers.
	m_context->addEventHandler< PostBuildEvent >(this, &SceneEditorPage::eventContextPostBuild);
	m_context->addEventHandler< ui::SelectionChangeEvent >(this, &SceneEditorPage::eventContextSelect);
	m_context->addEventHandler< PreModifyEvent >(this, &SceneEditorPage::eventContextPreModify);
	m_context->addEventHandler< PostModifyEvent >(this, &SceneEditorPage::eventContextPostModify);
	m_context->addEventHandler< CameraMovedEvent >(this, &SceneEditorPage::eventContextCameraMoved);

	// Finally realize the scene.
	createSceneAsset();
	updateScene();
	createInstanceGrid();
	createControllerEditor();
	updatePropertyObject();
	updateStatusBar();

	return true;
}

void SceneEditorPage::destroy()
{
	// Destroy controller editor.
	if (m_context->getControllerEditor())
		m_context->getControllerEditor()->destroy();

	// Destroy panels.
	m_site->destroyAdditionalPanel(m_entityPanel);
	m_site->destroyAdditionalPanel(m_tabMisc);
	m_site->destroyAdditionalPanel(m_controllerPanel);

	// Destroy widgets.
	safeDestroy(m_editPanel);
	safeDestroy(m_editControl);
	safeDestroy(m_entityPanel);
	safeDestroy(m_tabMisc);
	safeDestroy(m_entityMenu);
	safeDestroy(m_entityMenuExternal);
	safeDestroy(m_controllerPanel);
	safeDestroy(m_entityToolBar);
	safeDestroy(m_instanceGrid);

	// Destroy physics manager.
	if (m_context->getPhysicsManager())
		m_context->getPhysicsManager()->destroy();

	m_context->destroy();
	m_context = 0;
}

void SceneEditorPage::activate()
{
	m_editControl->setVisible(true);
}

void SceneEditorPage::deactivate()
{
	m_editControl->setVisible(false);
}

bool SceneEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	// Get index of view where user dropped instance.
	uint32_t viewIndex;
	if (!m_editControl->getViewIndex(position, viewIndex))
		return false;

	Ref< world::EntityData > entityData;

	// Check profiles if any can convert instance into an entity data.
	const RefArray< ISceneEditorProfile >& editorProfiles = m_context->getEditorProfiles();
	for (RefArray< ISceneEditorProfile >::const_iterator i = editorProfiles.begin(); i != editorProfiles.end(); ++i)
	{
		if ((entityData = (*i)->createEntityData(m_context, instance)) != 0)
			break;
	}

	if (entityData)
	{
		Ref< EntityAdapter > parentGroupAdapter;

		// Get selected items, must be a single item.
		RefArray< ui::custom::GridRow > selectedRows;
		if (m_instanceGrid->getRows(selectedRows, ui::custom::GridView::GfDescendants | ui::custom::GridView::GfSelectedOnly) == 1)
		{
			Ref< EntityAdapter > selectedEntity = selectedRows[0]->getData< EntityAdapter >(L"ENTITY");
			T_ASSERT (selectedEntity);

			parentGroupAdapter = selectedEntity->getParentGroup();
		}

		// Ensure drop is valid.
		if (!parentGroupAdapter)
		{
			log::error << L"Unable to drop entity; no layer or group selected" << Endl;
			return false;
		}
		if (parentGroupAdapter->isLocked(true))
		{
			log::error << L"Unable to drop entity; layer or group is locked" << Endl;
			return false;
		}

		// Ensure group is selected when editing a prefab.
		Object* documentObject = m_context->getDocument()->getObject(0);
		T_ASSERT (documentObject);

		if (world::EntityData* entityData = dynamic_type_cast< world::EntityData* >(documentObject))
		{
			if (parentGroupAdapter->isLayer())
			{
				log::error << L"Unable to drop entity; no prefab group selected" << Endl;
				return false;
			}
		}

		// Issue automatic build of dropped entity just in case the
		// entity hasn't been built.
		if (m_editor->getSettings()->getProperty< bool >(L"SceneEditor.BuildWhenDrop", true))
			m_editor->buildAsset(instance->getGuid(), false);

		m_context->getDocument()->push();

		// Create instance and adapter.
		Ref< EntityAdapter > entityAdapter = new EntityAdapter(m_context);
		entityAdapter->prepare(entityData, 0, 0);

		// Place instance in front of perspective camera.
		const Camera* camera = m_context->getCamera(viewIndex);
		T_ASSERT (camera);

		Matrix44 Mworld = camera->getWorld().toMatrix44() * translate(0.0f, 0.0f, 4.0f);
		entityAdapter->setTransform(Transform(Mworld.translation()));

		// Finally add adapter to parent group.
		parentGroupAdapter->addChild(entityAdapter);

		updateScene();
		createInstanceGrid();

		// Select entity.
		m_context->selectAllEntities(false);
		m_context->selectEntity(entityAdapter);
		m_context->raiseSelect();
	}
	else
		return false;

	return true;
}

bool SceneEditorPage::handleCommand(const ui::Command& command)
{
	bool result = true;

	if (command == L"Editor.PropertiesChanging")
		m_context->getDocument()->push();
	if (command == L"Editor.PropertiesChanged")
	{
		updateScene();
		createInstanceGrid();

		// Notify controller editor as well.
		Ref< ISceneControllerEditor > controllerEditor = m_context->getControllerEditor();
		if (controllerEditor)
			controllerEditor->propertiesChanged();

		m_context->raiseSelect();
	}
	else if (command == L"Editor.Undo")
	{
		if (!m_context->getDocument()->undo())
			return false;

		createSceneAsset();
		updateScene();
		createInstanceGrid();

		m_context->raiseSelect();
	}
	else if (command == L"Editor.Redo")
	{
		if (!m_context->getDocument()->redo())
			return false;

		createSceneAsset();
		updateScene();
		createInstanceGrid();

		m_context->raiseSelect();
	}
	else if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		RefArray< EntityAdapter > selectedEntities;
		if (!m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			return false;

		m_context->getDocument()->push();

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
			createInstanceGrid();

			m_context->raiseSelect();
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

		const RefArray< world::EntityData >& entityData = entityClipboardData->getEntityData();
		if (entityData.empty())
			return false;

		m_context->getDocument()->push();

		// Create new instances and adapters for each entity found in clipboard.
		for (RefArray< world::EntityData >::const_iterator i = entityData.begin(); i != entityData.end(); ++i)
		{
			Ref< EntityAdapter > entityAdapter = new EntityAdapter(m_context);
			entityAdapter->prepare(*i, 0, 0);
			parentEntity->addChild(entityAdapter);
		}

		updateScene();
		createInstanceGrid();

		m_context->raiseSelect();
	}
	else if (command == L"Editor.Delete")
	{
		RefArray< EntityAdapter > selectedEntities;
		if (!m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			return false;

		m_context->getDocument()->push();

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
			else if ((*i)->isLayer())
			{
				RefArray< world::LayerEntityData > layers = m_context->getSceneAsset()->getLayers();
				layers.remove(checked_type_cast< world::LayerEntityData*, false >((*i)->getEntityData()));
				m_context->getSceneAsset()->setLayers(layers);
				removedCount++;
			}
		}

		if (removedCount)
		{
			updateScene();
			createInstanceGrid();

			m_context->raiseSelect();
		}
	}
	else if (command == L"Editor.SelectAll")
	{
		m_context->selectAllEntities();
		m_context->raiseSelect();
	}
	else if (command == L"Scene.Editor.AddEntity")
		result = addEntity(0);
	else if (command == L"Scene.Editor.AddGroupEntity")
		result = addEntity(&type_of< world::GroupEntityData >());
	else if (command == L"Scene.Editor.MoveToEntity")
		result = moveToEntity();
	else if (command == L"Scene.Editor.FilterEntity")
	{
		if (m_buttonFilterEntity->isToggled())
			m_entityFilterType = m_editor->browseType(makeTypeInfoSet< world::Entity, world::IEntityComponent >());
		else
			m_entityFilterType = 0;

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.EnlargeGuide")
	{
		float guideSize = m_context->getGuideSize();
		m_context->setGuideSize(guideSize + 0.5f);
	}
	else if (command == L"Scene.Editor.ShrinkGuide")
	{
		float guideSize = m_context->getGuideSize();
		m_context->setGuideSize(std::max(guideSize - 0.5f, 0.5f));
	}
	else if (command == L"Scene.Editor.ResetGuide")
		m_context->setGuideSize(2.0f);
	else if (command == L"Scene.Editor.FindInDatabase")
	{
		RefArray< EntityAdapter > selectedEntities;
		if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) != 1)
			return false;

		Guid externalGuid;
		if (selectedEntities[0]->getExternalGuid(externalGuid))
		{
			Ref< db::Instance > externalInstance = m_context->getEditor()->getSourceDatabase()->getInstance(externalGuid);
			if (externalInstance)
				m_context->getEditor()->highlightInstance(externalInstance);
		}
	}
	else if (command == L"Scene.Editor.LockEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);

		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
			(*i)->setLocked(true);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.UnlockEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);

		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
			(*i)->setLocked(false);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.UnlockAllEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfDescendants);

		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
			(*i)->setLocked(false);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.ShowEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);

		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
			(*i)->setVisible(true);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.ShowAllEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfDescendants);

		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
			(*i)->setVisible(true);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.HideEntities")
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);

		for (RefArray< EntityAdapter >::iterator i = selectedEntities.begin(); i != selectedEntities.end(); ++i)
			(*i)->setVisible(false);

		createInstanceGrid();
	}
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

void SceneEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (!m_context || database == m_editor->getSourceDatabase())
		return;

	// Flush resource from manager.
	m_context->getResourceManager()->reload(eventId, false);

	// Check if guid is used as an external reference.
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
		createInstanceGrid();
	}
}

bool SceneEditorPage::createSceneAsset()
{
	Object* documentObject = m_context->getDocument()->getObject(0);
	if (!documentObject)
		return false;

	if (SceneAsset* sceneAsset = dynamic_type_cast< SceneAsset* >(documentObject))
		m_context->setSceneAsset(sceneAsset);
	else if (world::EntityData* entityData = dynamic_type_cast< world::EntityData* >(documentObject))
	{
		Ref< SceneAsset > sceneAsset = m_context->getSourceDatabase()->getObjectReadOnly< SceneAsset >(c_guidWhiteRoomScene);
		if (!sceneAsset)
			return false;

		const RefArray< world::LayerEntityData >& layers = sceneAsset->getLayers();
		T_ASSERT (layers.size() >= 2);

		layers[1]->addEntityData(entityData);

		m_context->setSceneAsset(sceneAsset);
	}
	else
		return false;

	return true;
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
			RefArray< const ISceneControllerEditorFactory > controllerEditorFactories;
			Ref< ISceneControllerEditor > controllerEditor;

			// Create controller editor factories.
			const RefArray< ISceneEditorProfile >& profiles = m_context->getEditorProfiles();
			for (RefArray< ISceneEditorProfile >::const_iterator i = profiles.begin(); i != profiles.end(); ++i)
				(*i)->createControllerEditorFactories(m_context, controllerEditorFactories);

			for (RefArray< const ISceneControllerEditorFactory >::iterator i = controllerEditorFactories.begin(); i != controllerEditorFactories.end(); ++i)
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
				T_DEBUG(L"Unable to find controller editor for type \"" << type_name(controllerData) << L"\"");
		}
	}
}

void SceneEditorPage::updateScene()
{
	m_context->buildEntities();

	Ref< scene::SceneAsset > sceneAsset = m_context->getSceneAsset();
	if (sceneAsset)
	{
		// Check if any scene settings has changed.
		bool needUpdate = false;

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

Ref< ui::custom::GridRow > SceneEditorPage::createInstanceGridRow(EntityAdapter* entityAdapter)
{
	if (m_entityFilterType && !filterIncludeEntity(*m_entityFilterType, entityAdapter))
		return 0;

	Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);
	row->setData(L"ENTITY", entityAdapter);
	row->setState(
		(entityAdapter->isSelected() ? ui::custom::GridRow::RsSelected : 0) |
		(entityAdapter->isExpanded() ? ui::custom::GridRow::RsExpanded : 0)
	);

	std::wstring entityName = entityAdapter->getName();
	if (entityName.empty())
		entityName = i18n::Text(L"SCENE_EDITOR_UNNAMED_ENTITY");

	if (entityAdapter->isExternal())
		row->add(new ui::custom::GridItem(entityName, m_instanceGridFontBold/*, 1*/));
	else if (entityAdapter->isLayer())
	{
		row->add(new ui::custom::GridItem(entityName, m_instanceGridFontHuge/*, 4*/));
		row->setMinimumHeight(ui::scaleBySystemDPI(32));
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
	if (
		!entityAdapter->isExternal() &&
		!entityAdapter->isChildrenPrivate()
	)
	{
		const RefArray< EntityAdapter >& children = entityAdapter->getChildren();
		for (RefArray< EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
		{
			Ref< ui::custom::GridRow > child = createInstanceGridRow(*i);
			if (child)
				row->addChild(child);
		}
	}

	return row;
}

void SceneEditorPage::createInstanceGrid()
{
	m_instanceGrid->removeAllRows();

	const RefArray< EntityAdapter >& layerEntityAdapters = m_context->getLayerEntityAdapters();
	for (RefArray< EntityAdapter >::const_iterator j = layerEntityAdapters.begin(); j != layerEntityAdapters.end(); ++j)
	{
		Ref< ui::custom::GridRow > entityRow = createInstanceGridRow(*j);
		if (entityRow)
			m_instanceGrid->addRow(entityRow);
	}

	m_instanceGrid->update();
}

void SceneEditorPage::updateInstanceGridRow(ui::custom::GridRow* row)
{
	EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");

	row->setState(
		(entityAdapter->isSelected() ? ui::custom::GridRow::RsSelected : 0) |
		(entityAdapter->isExpanded() ? ui::custom::GridRow::RsExpanded : 0)
	);

	const RefArray< ui::custom::GridRow >& childRows = row->getChildren();
	for (RefArray< ui::custom::GridRow >::const_iterator i = childRows.begin(); i != childRows.end(); ++i)
		updateInstanceGridRow(*i);
}

void SceneEditorPage::updateInstanceGrid()
{
	const RefArray< ui::custom::GridRow >& rows = m_instanceGrid->getRows();
	for (RefArray< ui::custom::GridRow >::const_iterator i = rows.begin(); i != rows.end(); ++i)
		updateInstanceGridRow(*i);

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
		m_site->setPropertyObject(m_context->getDocument()->getObject(0));
}

void SceneEditorPage::updateStatusBar()
{
	const Camera* camera = m_context->getCamera(0);
	T_ASSERT (camera);

	Vector4 position = camera->getPosition();
	Vector4 angles = camera->getOrientation().toEulerAngles();

	StringOutputStream ss;
	ss.setDecimals(2);
	ss << position.x() << L", " << position.y() << L", " << position.z() << L"     ";
	ss << rad2deg(angles.x()) << L", " << rad2deg(angles.y()) << L", " << rad2deg(angles.z()) << L" deg" << L"     ";
	ss << m_context->getEntityCount() << L" entities";

	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
		ss << L"     " << selectedEntities[0]->getPath();

	m_statusBar->setText(ss.str());
}

bool SceneEditorPage::addEntity(const TypeInfo* entityType)
{
	Ref< EntityAdapter > parentGroupAdapter;

	// Get selected entity, must be a single item.
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
		parentGroupAdapter = selectedEntities[0]->getParentGroup();

	// Ensure add is valid.
	if (!parentGroupAdapter)
		return false;
	if (parentGroupAdapter->isLocked(true))
	{
		log::error << L"Unable to add entity; layer or group is locked" << Endl;
		return false;
	}

	// Select type of entity to create.
	if (!entityType)
	{
		if ((entityType = m_context->getEditor()->browseType(makeTypeInfoSet< world::EntityData >())) == 0)
			return false;
	}

	Ref< world::EntityData > entityData = checked_type_cast< world::EntityData* >(entityType->createInstance());
	T_ASSERT (entityData);

	m_context->getDocument()->push();

	Ref< EntityAdapter > entityAdapter = new EntityAdapter(m_context);
	entityAdapter->prepare(entityData, 0, 0);
	parentGroupAdapter->addChild(entityAdapter);

	updateScene();
	createInstanceGrid();

	return true;
}

bool SceneEditorPage::moveToEntity()
{
	RefArray< EntityAdapter > selectedEntities;
	if (m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants) == 1)
		m_context->moveToEntityAdapter(selectedEntities[0]);
	else
		m_context->moveToEntityAdapter(0);
	return true;
}

void SceneEditorPage::eventEntityToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void SceneEditorPage::eventGuideClick(ui::custom::GridColumnClickEvent* event)
{
	if (event->getColumn() == 1)
	{
		ui::custom::GridRow* row = event->getRow();
		std::wstring id = row->get(0)->getText();

		bool shouldDraw = !m_context->shouldDrawGuide(id);
		m_context->setDrawGuide(id, shouldDraw);

		row->set(1, new ui::custom::GridItem(shouldDraw ? m_imageVisible : m_imageHidden));
		m_gridGuides->requestUpdate();

		m_editor->checkoutGlobalSettings()->setProperty< PropertyBoolean >(L"SceneEditor.Guides/" + id, shouldDraw);
		m_editor->commitGlobalSettings();
	}
}

void SceneEditorPage::eventInstanceSelect(ui::SelectionChangeEvent* event)
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
	m_context->raiseSelect();
}

void SceneEditorPage::eventInstanceExpand(ui::custom::GridRowStateChangeEvent* event)
{
	ui::custom::GridRow* row = event->getRow();
	EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
	entityAdapter->setExpanded((row->getState() & ui::custom::GridRow::RsExpanded) != 0);
}

void SceneEditorPage::eventInstanceButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() == ui::MbtRight)
	{
		RefArray< EntityAdapter > selectedEntities;
		m_context->getEntities(selectedEntities, SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);

		Ref< ui::MenuItem > selectedItem;
		if (selectedEntities.size() == 1 && selectedEntities[0]->isExternal())
			selectedItem = m_entityMenuExternal->show(m_instanceGrid, event->getPosition());
		else
			selectedItem = m_entityMenu->show(m_instanceGrid, event->getPosition());

		if (selectedItem)
		{
			if (handleCommand(selectedItem->getCommand()))
				event->consume();
		}
	}
}

void SceneEditorPage::eventInstanceClick(ui::custom::GridColumnClickEvent* event)
{
	if (event->getColumn() == 1)
	{
		ui::custom::GridRow* row = event->getRow();
		ui::custom::GridItem* item = row->get(1);

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
	else if (event->getColumn() == 2)
	{
		ui::custom::GridRow* row = event->getRow();
		ui::custom::GridItem* item = row->get(2);

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

void SceneEditorPage::eventContextPostBuild(PostBuildEvent* event)
{
	createInstanceGrid();
	updateStatusBar();
}

void SceneEditorPage::eventContextSelect(ui::SelectionChangeEvent* event)
{
	updateInstanceGrid();
	updatePropertyObject();
	updateStatusBar();
}

void SceneEditorPage::eventContextPreModify(PreModifyEvent* event)
{
	m_context->getDocument()->push();
}

void SceneEditorPage::eventContextPostModify(PostModifyEvent* event)
{
	updatePropertyObject();
}

void SceneEditorPage::eventContextCameraMoved(CameraMovedEvent* event)
{
	updateStatusBar();
}

	}
}
