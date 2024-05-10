/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Settings/PropertyArray.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/PropertiesView.h"
#include "I18N/Text.h"
#include "Physics/PhysicsManager.h"
#include "Render/IRenderSystem.h"
#include "Resource/ResourceManager.h"
#include "Scene/Scene.h"
#include "Scene/SceneFactory.h"
#include "Scene/Editor/Camera.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/EntityClipboardData.h"
#include "Scene/Editor/EntityDependencyInvestigator.h"
#include "Scene/Editor/IWorldComponentEditorFactory.h"
#include "Scene/Editor/IWorldComponentEditor.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/SceneEditorPage.h"
#include "Scene/Editor/ScenePreviewControl.h"
#include "Scene/Editor/Utilities.h"
#include "Scene/Editor/Events/CameraMovedEvent.h"
#include "Scene/Editor/Events/MeasurementEvent.h"
#include "Scene/Editor/Events/PostBuildEvent.h"
#include "Scene/Editor/Events/PostFrameEvent.h"
#include "Scene/Editor/Events/PostModifyEvent.h"
#include "Scene/Editor/Events/PreModifyEvent.h"
#include "Script/IScriptManager.h"
#include "Script/ScriptFactory.h"
#include "Ui/Application.h"
#include "Ui/BackgroundWorkerDialog.h"
#include "Ui/Clipboard.h"
#include "Ui/Container.h"
#include "Ui/FloodLayout.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MessageBox.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/TableLayout.h"
#include "Ui/InputDialog.h"
#include "Ui/PropertyList/NumericPropertyItem.h"
#include "Ui/PropertyList/PropertyContentChangeEvent.h"
#include "Ui/StatusBar/StatusBar.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/GridView/GridView.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridColumnClickEvent.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowStateChangeEvent.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridItemContentChangeEvent.h"
#include "World/Entity.h"
#include "World/EntityData.h"
#include "World/EntityFactory.h"
#include "World/IEntityComponent.h"
#include "World/IEntityComponentData.h"
#include "World/IWorldComponentData.h"
#include "World/WorldRenderSettings.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor::scene
{
	namespace
	{

const Guid c_guidWhiteRoomScene(L"{473467B0-835D-EF45-B308-E3C3C5B0F226}");

void renameIds(ISerializable* object, const SmallMap< Guid, Guid >& renamedMap)
{
	Ref< Reflection > reflection = Reflection::create(object);

	// Rename all id;s in this object first.
	RefArray< ReflectionMember > idMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmPrimitiveGuid >()), idMembers);
	for (auto idMember : idMembers)
	{
		auto id = static_cast< RfmPrimitiveGuid* >(idMember.ptr());
		auto it = renamedMap.find(id->get());
		if (it != renamedMap.end())
			id->set(it->second);
	}

	// Recurse with child objects.
	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);
	for (auto objectMember : objectMembers)
	{
		auto object = static_cast< RfmObject* >(objectMember.ptr());
		renameIds(object->get(), renamedMap);
	}

	reflection->apply(object);
}

bool isChildEntitySelected(const EntityAdapter* entityAdapter)
{
	for (auto child : entityAdapter->getChildren())
	{
		if (child->isSelected())
			return true;
		if (isChildEntitySelected(child))
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

	if (entityAdapter->getComponent(entityOrComponentType) != nullptr)
		return true;

	for (auto child : entityAdapter->getChildren())
	{
		if (filterIncludeEntity(entityOrComponentType, child))
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
{
}

bool SceneEditorPage::create(ui::Container* parent)
{
	std::set< std::wstring > guideIds;

	// Get render system from store.
	render::IRenderSystem* renderSystem = m_editor->getObjectStore()->get< render::IRenderSystem >();
	if (!renderSystem)
		return false;

	// Get physics manager type.
	const std::wstring physicsManagerTypeName = m_editor->getSettings()->getProperty< std::wstring >(L"SceneEditor.PhysicsManager");
	const TypeInfo* physicsManagerType = TypeInfo::find(physicsManagerTypeName.c_str());
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

	// Create script context.
	Ref< script::IScriptManager > scriptManager = m_editor->getObjectStore()->get< script::IScriptManager >();
	if (!scriptManager)
	{
		log::error << L"Unable to create scene editor; failed to get script manager." << Endl;
		return false;
	}

	Ref< script::IScriptContext > scriptContext = scriptManager->createContext(false);
	if (!scriptContext)
	{
		log::error << L"Unable to create scene editor; failed to create script context." << Endl;
		return false;
	}

	// Create resource manager.
	Ref< resource::IResourceManager > resourceManager = new resource::ResourceManager(
		m_editor->getOutputDatabase(),
		m_editor->getSettings()->getProperty< bool >(L"Resource.Verbose", false)
	);

	// Create editor context.
	m_context = new SceneEditorContext(
		m_editor,
		m_document,
		m_editor->getOutputDatabase(),
		m_editor->getSourceDatabase(),
		resourceManager,
		renderSystem,
		physicsManager,
		scriptContext
	);

	// Create profiles, plugins, resource factories, entity editors and guide ids.
	for (auto profileType : type_of< ISceneEditorProfile >().findAllOf())
	{
		Ref< ISceneEditorProfile > profile = dynamic_type_cast< ISceneEditorProfile* >(profileType->createInstance());
		if (!profile)
			continue;

		m_context->addEditorProfile(profile);

		RefArray< ISceneEditorPlugin > editorPlugins;
		profile->createEditorPlugins(m_context, editorPlugins);

		for (auto editorPlugin : editorPlugins)
			m_context->addEditorPlugin(editorPlugin);

		RefArray< const resource::IResourceFactory > resourceFactories;
		profile->createResourceFactories(m_context, resourceFactories);

		for (auto resourceFactory : resourceFactories)
			resourceManager->addFactory(resourceFactory);

		profile->getGuideDrawIds(guideIds);
	}

	// Create entity and component editor factories.
	m_context->createFactories();

	// Create scene instance resource factory, used by final render control etc.
	Ref< world::EntityFactory > entityFactory = new world::EntityFactory();
	for (auto editorProfile : m_context->getEditorProfiles())
	{
		RefArray< const world::IEntityFactory > entityFactories;
		editorProfile->createEntityFactories(m_context, entityFactories);
		for (auto factory : entityFactories)
			entityFactory->addFactory(factory);
	}
	m_context->getResourceManager()->addFactory(new SceneFactory(entityFactory));

	// Create editor panel.
	m_editPanel = new ui::Container();
	m_editPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0_ut, 0_ut));

	m_editControl = new ScenePreviewControl();
	if (!m_editControl->create(m_editPanel, m_context))
	{
		log::error << L"Unable to create scene editor; failed to scene preview." << Endl;
		safeDestroy(physicsManager);
		return false;
	}

	m_statusBar = new ui::StatusBar();
	m_statusBar->create(m_editPanel, ui::WsDoubleBuffer);
	m_statusBar->addColumn(0);	// Position
	m_statusBar->addColumn(0);	// Orientation
	m_statusBar->addColumn(0);	// Entity count
	m_statusBar->addColumn(0);	// Time
	m_statusBar->addColumn(0);	// Selected entity

	// Create entity panel.
	m_entityPanel = new ui::Container();
	m_entityPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));
	m_entityPanel->setText(i18n::Text(L"SCENE_EDITOR_ENTITIES"));

	m_entityMenuDefault = new ui::Menu();
	m_entityMenuDefault->add(new ui::MenuItem(ui::Command(L"Scene.Editor.AddComponent"), i18n::Text(L"SCENE_EDITOR_ADD_COMPONENT")));
	m_entityMenuDefault->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY")));
	m_entityMenuDefault->add(new ui::MenuItem(L"-"));
	m_entityMenuDefault->add(new ui::MenuItem(ui::Command(L"Scene.Editor.CreateExternal"), i18n::Text(L"SCENE_EDITOR_CREATE_EXTERNAL")));
	m_entityMenuDefault->add(new ui::MenuItem(L"-"));
	m_entityMenuDefault->add(new ui::MenuItem(ui::Command(L"Scene.Editor.MoveToEntity"), i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY")));

	m_entityMenuGroup = new ui::Menu();
	m_entityMenuGroup->add(new ui::MenuItem(ui::Command(L"Scene.Editor.AddComponent"), i18n::Text(L"SCENE_EDITOR_ADD_COMPONENT")));
	m_entityMenuGroup->add(new ui::MenuItem(ui::Command(L"Scene.Editor.AddEntity"), i18n::Text(L"SCENE_EDITOR_ADD_ENTITY")));
	m_entityMenuGroup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY")));
	m_entityMenuGroup->add(new ui::MenuItem(L"-"));
	m_entityMenuGroup->add(new ui::MenuItem(ui::Command(L"Scene.Editor.CreateExternal"), i18n::Text(L"SCENE_EDITOR_CREATE_EXTERNAL")));
	m_entityMenuGroup->add(new ui::MenuItem(L"-"));
	m_entityMenuGroup->add(new ui::MenuItem(ui::Command(L"Scene.Editor.MoveToEntity"), i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY")));

	m_entityMenuExternal = new ui::Menu();
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Scene.Editor.FindInDatabase"), i18n::Text(L"SCENE_EDITOR_FIND_IN_DATABASE")));
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY")));
	m_entityMenuExternal->add(new ui::MenuItem(L"-"));
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Scene.Editor.ResolveExternal"), i18n::Text(L"SCENE_EDITOR_RESOLVE_EXTERNAL")));
	m_entityMenuExternal->add(new ui::MenuItem(L"-"));
	m_entityMenuExternal->add(new ui::MenuItem(ui::Command(L"Scene.Editor.MoveToEntity"), i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY")));

	m_entityToolBar = new ui::ToolBar();
	m_entityToolBar->create(m_entityPanel);
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.RemoveEntity"));
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.MoveToEntity"));
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.FilterEntity"));
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.MoveUpEntity"));
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.MoveDownEntity"));
	m_entityToolBar->addImage(new ui::StyleBitmap(L"Scene.LayerAdd"));
	m_entityToolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_REMOVE_ENTITY"), 0, ui::Command(L"Editor.Delete")));
	m_entityToolBar->addItem(new ui::ToolBarSeparator());
	m_entityToolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_MOVE_TO_ENTITY"), 1, ui::Command(L"Scene.Editor.MoveToEntity")));
	m_buttonFilterEntity = new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_FILTER_ENTITY"), 2, ui::Command(L"Scene.Editor.FilterEntity"), ui::ToolBarButton::BsDefaultToggle);
	m_entityToolBar->addItem(m_buttonFilterEntity);
	m_entityToolBar->addItem(new ui::ToolBarSeparator());
	m_entityToolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_MOVE_UP"), 3, ui::Command(L"Scene.Editor.MoveUp")));
	m_entityToolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_MOVE_DOWN"), 4, ui::Command(L"Scene.Editor.MoveDown")));
	m_entityToolBar->addItem(new ui::ToolBarSeparator());
	m_entityToolBar->addItem(new ui::ToolBarButton(i18n::Text(L"SCENE_EDITOR_NEW_LAYER"), 5, ui::Command(L"Scene.Editor.NewLayer")));
	m_entityToolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &SceneEditorPage::eventEntityToolClick);

	m_imageHidden = new ui::StyleBitmap(L"Scene.LayerHidden");
	m_imageVisible = new ui::StyleBitmap(L"Scene.LayerVisible");
	m_imageLocked = new ui::StyleBitmap(L"Scene.LayerLocked");
	m_imageUnlocked = new ui::StyleBitmap(L"Scene.LayerUnlocked");

	m_instanceGrid = new ui::GridView();
	m_instanceGrid->create(m_entityPanel, ui::GridView::WsMultiSelect | ui::GridView::WsAutoEdit | ui::WsDoubleBuffer);
	m_instanceGrid->addColumn(new ui::GridColumn(L"", 200_ut, true));
	m_instanceGrid->addColumn(new ui::GridColumn(L"", 30_ut));
	m_instanceGrid->addColumn(new ui::GridColumn(L"", 30_ut));
	m_instanceGrid->addEventHandler< ui::SelectionChangeEvent >(this, &SceneEditorPage::eventInstanceSelect);
	m_instanceGrid->addEventHandler< ui::GridRowStateChangeEvent >(this, &SceneEditorPage::eventInstanceExpand);
	m_instanceGrid->addEventHandler< ui::MouseButtonDownEvent >(this, &SceneEditorPage::eventInstanceButtonDown);
	m_instanceGrid->addEventHandler< ui::GridColumnClickEvent >(this, &SceneEditorPage::eventInstanceClick);
	m_instanceGrid->addEventHandler< ui::GridItemContentChangeEvent >(this, &SceneEditorPage::eventInstanceRename);

	m_instanceGridFontBold = new ui::Font(m_instanceGrid->getFont());
	m_instanceGridFontBold->setBold(true);

	m_instanceGridFontHuge = new ui::Font(m_instanceGrid->getFont());
	m_instanceGridFontHuge->setBold(true);
	m_instanceGridFontHuge->setSize(14_ut);

	m_site->createAdditionalPanel(m_entityPanel, 400_ut, false);

	m_tabMisc = new ui::Tab();
	m_tabMisc->create(parent, ui::Tab::WsLine);
	m_tabMisc->setText(i18n::Text(L"SCENE_EDITOR_MISC"));

	// Create properties page.
	Ref< ui::TabPage > tabPageProperties = new ui::TabPage();
	tabPageProperties->create(m_tabMisc, i18n::Text(L"TITLE_PROPERTIES"), new ui::FloodLayout());

	m_propertiesView = m_site->createPropertiesView(tabPageProperties);
	m_propertiesView->addEventHandler< ui::ContentChangingEvent >(this, &SceneEditorPage::eventPropertiesChanging);
	m_propertiesView->addEventHandler< ui::ContentChangeEvent >(this, &SceneEditorPage::eventPropertiesChanged);

	// Create dependency panel.
	Ref< ui::TabPage > tabPageDependencies = new ui::TabPage();
	tabPageDependencies->create(m_tabMisc, i18n::Text(L"SCENE_EDITOR_DEPENDENCY_INVESTIGATOR"), new ui::FloodLayout());

	m_entityDependencyPanel = new EntityDependencyInvestigator(m_context);
	m_entityDependencyPanel->create(tabPageDependencies);

	// Create guide visibility panel.
	Ref< ui::TabPage > tabPageGuides = new ui::TabPage();
	tabPageGuides->create(m_tabMisc, i18n::Text(L"SCENE_EDITOR_GUIDES"), new ui::FloodLayout());

	m_gridGuides = new ui::GridView();
	m_gridGuides->create(tabPageGuides, ui::WsDoubleBuffer | ui::WsTabStop);
	m_gridGuides->addColumn(new ui::GridColumn(i18n::Text(L"SCENE_EDITOR_GUIDES_NAME"), 150_ut));
	m_gridGuides->addColumn(new ui::GridColumn(i18n::Text(L"SCENE_EDITOR_GUIDES_VISIBLE"), 50_ut));

	for (const auto& guideId : guideIds)
	{
		bool shouldDraw = m_editor->getSettings()->getProperty< bool >(L"SceneEditor.Guides/" + guideId, true);
		m_context->setDrawGuide(guideId, shouldDraw);

		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(guideId);
		row->add(shouldDraw ? m_imageVisible : m_imageHidden);
		m_gridGuides->addRow(row);
	}

	m_gridGuides->addEventHandler< ui::GridColumnClickEvent >(this, &SceneEditorPage::eventGuideClick);

	// Create measurements panel.
	Ref< ui::TabPage > tabPageMeasurements = new ui::TabPage();
	tabPageMeasurements->create(m_tabMisc, i18n::Text(L"SCENE_EDITOR_MEASUREMENTS"), new ui::FloodLayout());

	m_gridMeasurements = new ui::GridView();
	m_gridMeasurements->create(tabPageMeasurements, ui::WsDoubleBuffer | ui::WsTabStop);
	m_gridMeasurements->addColumn(new ui::GridColumn(L"", 50_ut));
	m_gridMeasurements->addColumn(new ui::GridColumn(i18n::Text(L"SCENE_EDITOR_MEASUREMENTS_NAME"), 150_ut));
	m_gridMeasurements->addColumn(new ui::GridColumn(i18n::Text(L"SCENE_EDITOR_MEASUREMENTS_DURATION"), 90_ut));

	// Create resources panel.
	Ref< ui::TabPage > tabPageResources = new ui::TabPage();
	tabPageResources->create(m_tabMisc, i18n::Text(L"SCENE_EDITOR_RESOURCES"), new ui::FloodLayout());

	m_gridResources = new ui::GridView();
	m_gridResources->create(tabPageResources, ui::WsDoubleBuffer | ui::WsTabStop);
	m_gridResources->addColumn(new ui::GridColumn(L"", 180_ut));
	m_gridResources->addColumn(new ui::GridColumn(L"", 150_ut));

	// Add pages.
	m_tabMisc->addPage(tabPageProperties);
	m_tabMisc->addPage(tabPageDependencies);
	m_tabMisc->addPage(tabPageGuides);
	m_tabMisc->addPage(tabPageMeasurements);
	m_tabMisc->addPage(tabPageResources);
	m_tabMisc->setActivePage(tabPageProperties);

	m_site->createAdditionalPanel(m_tabMisc, 400_ut, false);

	// Create controller panel.
	m_controllerPanel = new ui::Container();
	m_controllerPanel->create(parent, ui::WsNone, new ui::FloodLayout());
	m_controllerPanel->setText(i18n::Text(L"SCENE_EDITOR_CONTROLLER"));

	m_site->createAdditionalPanel(m_controllerPanel, 140_ut, true);

	// Create the scene, loads textures etc, using a background job since it might take significant amount of time.
	Ref< Job > job = JobManager::getInstance().add([&]() {
		createSceneAsset();
		updateScene();
	});

	// Show a dialog if processing seems to take more than N second(s).
	ui::BackgroundWorkerDialog dialog;
	dialog.create(parent->getAncestor(), i18n::Text(L"SCENE_EDITOR_LOAD_TITLE"), i18n::Text(L"SCENE_EDITOR_LOAD_MESSAGE"), false);
	dialog.execute(job, nullptr);
	dialog.destroy();

	// Context event handlers.
	m_context->addEventHandler< PostBuildEvent >(this, &SceneEditorPage::eventContextPostBuild);
	m_context->addEventHandler< ui::SelectionChangeEvent >(this, &SceneEditorPage::eventContextSelect);
	m_context->addEventHandler< PreModifyEvent >(this, &SceneEditorPage::eventContextPreModify);
	m_context->addEventHandler< PostModifyEvent >(this, &SceneEditorPage::eventContextPostModify);
	m_context->addEventHandler< CameraMovedEvent >(this, &SceneEditorPage::eventContextCameraMoved);
	m_context->addEventHandler< PostFrameEvent >(this, &SceneEditorPage::eventContextPostFrame);
	m_context->addEventHandler< MeasurementEvent >(this, &SceneEditorPage::eventContextMeasurement);

	// Load last used camera transforms.
	for (int32_t i = 0; i < 4; ++i)
	{
		std::wstring str = m_editor->getSettings()->getProperty< std::wstring >(L"SceneEditor.LastCameras/" + m_context->getDocument()->getInstance(0)->getGuid().format() + L"/" + toString(i));
		std::vector< float > values;
		Split< std::wstring, float >::any(str, L",", values);
		if (values.size() >= 7)
		{
			m_context->getCamera(i)->setPosition(Vector4(values[0], values[1], values[2], 1.0f));
			m_context->getCamera(i)->setOrientation(Quaternion(values[3], values[4], values[5], values[6]));
		}
	}

	// Finally realize the scene.
	createInstanceGrid();
	createControllerEditor();
	updatePropertyObject();
	updateStatusBar();

	// Frame entity in view.
	if (auto entityData = dynamic_type_cast< world::EntityData* >(m_context->getDocument()->getObject(0)))
	{
		for (auto entityAdapter : m_context->getEntities())
		{
			if (entityAdapter->getEntityData() == entityData)
				m_context->moveToEntityAdapter(entityAdapter);
		}
	}
	return true;
}

void SceneEditorPage::destroy()
{
	// Save cameras.
	auto settings = m_editor->checkoutGlobalSettings();
	if (settings)
	{
		for (int32_t i = 0; i < 4; ++i)
		{
			auto p = m_context->getCamera(i)->getPosition();
			auto o = m_context->getCamera(i)->getOrientation();
			settings->setProperty< PropertyString >(
				L"SceneEditor.LastCameras/" + m_context->getDocument()->getInstance(0)->getGuid().format() + L"/" + toString(i),
				str(L"%f, %f, %f, %f, %f, %f, %f", (float)p.x(), (float)p.y(), (float)p.z(), (float)o.e.x(), (float)o.e.y(), (float)o.e.z(), (float)o.e.w())
			);
		}
		m_editor->commitGlobalSettings();
	}

	// Destroy controller editor.
	if (m_context->getControllerEditor())
		m_context->getControllerEditor()->destroy();

	// Destroy panels.
	if (m_entityPanel)
		m_site->destroyAdditionalPanel(m_entityPanel);
	if (m_tabMisc)
		m_site->destroyAdditionalPanel(m_tabMisc);
	if (m_controllerPanel)
		m_site->destroyAdditionalPanel(m_controllerPanel);

	// Destroy widgets.
	safeDestroy(m_editPanel);
	safeDestroy(m_editControl);
	safeDestroy(m_entityPanel);
	safeDestroy(m_tabMisc);
	safeDestroy(m_controllerPanel);
	safeDestroy(m_entityToolBar);
	safeDestroy(m_instanceGrid);

	// Destroy physics manager.
	if (m_context->getPhysicsManager())
		m_context->getPhysicsManager()->destroy();

	safeDestroy(m_context);
	m_site = nullptr;
}

bool SceneEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	// Get index of view where user dropped instance.
	uint32_t viewIndex;
	if (!m_editControl->getViewIndex(position, viewIndex))
		return false;

	Ref< world::EntityData > entityData;

	// Check profiles if any can convert instance into an entity data.
	for (auto editorProfile : m_context->getEditorProfiles())
	{
		if ((entityData = editorProfile->createEntityData(m_context, instance)) != nullptr)
			break;
	}

	if (entityData)
	{
		Ref< EntityAdapter > parentGroupAdapter;

		// Get selected items, must be a single item.
		RefArray< ui::GridRow > selectedRows = m_instanceGrid->getRows(ui::GridView::GfDescendants | ui::GridView::GfSelectedOnly);
		if (selectedRows.size() == 1)
		{
			Ref< EntityAdapter > selectedEntity = selectedRows[0]->getData< EntityAdapter >(L"ENTITY");
			T_ASSERT(selectedEntity);

			parentGroupAdapter = selectedEntity->getParentGroup();
		}

		// Ensure drop is valid.
		if (!parentGroupAdapter)
		{
			log::error << L"Unable to drop entity; no layer or group selected." << Endl;
			return false;
		}
		if (parentGroupAdapter->isLocked(true))
		{
			log::error << L"Unable to drop entity; layer or group is locked." << Endl;
			return false;
		}

		// Ensure group is selected when editing a prefab.
		Object* documentObject = m_context->getDocument()->getObject(0);
		T_ASSERT(documentObject);

		// Issue automatic build of dropped entity just in case the
		// entity hasn't been built.
		if (m_editor->getSettings()->getProperty< bool >(L"SceneEditor.BuildWhenDrop", true))
			m_editor->buildAsset(instance->getGuid(), false);

		m_context->getDocument()->push();

		// Create instance and adapter.
		Ref< EntityAdapter > entityAdapter = new EntityAdapter(m_context);
		entityAdapter->prepare(entityData, nullptr);

		// Place instance in front of perspective camera.
		const Camera* camera = m_context->getCamera(viewIndex);
		T_ASSERT(camera);

		const Matrix44 Mworld = camera->getWorld().toMatrix44() * translate(0.0f, 0.0f, 4.0f);
		entityAdapter->setTransform(Transform(Mworld.translation()));

		// Finally add adapter to parent group.
		parentGroupAdapter->addChild(nullptr, entityAdapter);

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
	if (m_propertiesView->handleCommand(command))
		return true;
	
	bool result = true;

	if (command == L"Editor.Undo")
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
		RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
		if (selectedEntities.empty())
			return false;

		m_context->getDocument()->push();

		// Create clipboard data with all selected entities; remove entities from scene if we're cutting.
		Ref< EntityClipboardData > entityClipboardData = new EntityClipboardData();
		for (auto selectedEntity : selectedEntities)
		{
			entityClipboardData->addEntityData(selectedEntity->getEntityData());
			if (command == L"Editor.Cut")
			{
				Ref< EntityAdapter > parentGroup = selectedEntity->getParent();
				if (parentGroup->isGroup())
				{
					parentGroup->removeChild(selectedEntity);

					if (m_context->getControllerEditor())
						m_context->getControllerEditor()->entityRemoved(selectedEntity);

					selectedEntity->destroyEntity();
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
		RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
		if (selectedEntities.size() != 1)
			return false;

		Ref< EntityAdapter > parentEntity = selectedEntities[0]->getParentGroup();
		T_ASSERT(parentEntity);

		// Get clipboard data; ensure correct type.
		Ref< EntityClipboardData > entityClipboardData = dynamic_type_cast< EntityClipboardData* >(
			ui::Application::getInstance()->getClipboard()->getObject()
		);
		if (!entityClipboardData)
			return false;

		const RefArray< world::EntityData >& entityDatas = entityClipboardData->getEntityData();
		if (entityDatas.empty())
			return false;

		m_context->getDocument()->push();

		// Create new instances and adapters for each entity found in clipboard.
		for (auto entityData : entityDatas)
		{
			generateEntityIds(entityData);

			Ref< EntityAdapter > entityAdapter = new EntityAdapter(m_context);
			entityAdapter->prepare(entityData, nullptr);
			parentEntity->addChild(nullptr, entityAdapter);
		}

		updateScene();
		createInstanceGrid();

		m_context->raiseSelect();
	}
	else if (command == L"Editor.Delete")
	{
		RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
		if (selectedEntities.empty())
			return false;

		m_context->getDocument()->push();

		uint32_t removedCount = 0;
		for (auto selectedEntity : selectedEntities)
		{
			Ref< EntityAdapter > parentGroup = selectedEntity->getParent();
			if (parentGroup && parentGroup->isGroup())
			{
				parentGroup->removeChild(selectedEntity);

				if (m_context->getControllerEditor())
					m_context->getControllerEditor()->entityRemoved(selectedEntity);

				selectedEntity->destroyEntity();
				removedCount++;
			}
			else
			{
				RefArray< world::EntityData > layers = m_context->getSceneAsset()->getLayers();
				if (layers.remove(selectedEntity->getEntityData()))
				{
					m_context->getSceneAsset()->setLayers(layers);
					selectedEntity->destroyEntity();
					removedCount++;
				}
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
		m_context->selectAllEntities(true);
		m_context->raiseSelect();
		m_context->enqueueRedraw(nullptr);
	}
	else if (command == L"Editor.Unselect")
	{
		m_context->selectAllEntities(false);
		m_context->raiseSelect();
		m_context->enqueueRedraw(nullptr);
	}
	else if (command == L"Scene.Editor.AddComponent")
		result = addComponent();
	else if (command == L"Scene.Editor.CreateExternal")
		result = createExternal();
	else if (command == L"Scene.Editor.ResolveExternal")
		result = resolveExternal();
	else if (command == L"Scene.Editor.AddEntity")
		result = addEntity(nullptr);
	else if (command == L"Scene.Editor.MoveToEntity")
		result = moveToEntity();
	else if (command == L"Scene.Editor.MoveUp")
	{
		if ((result = moveUp()) == true)
		{
			updateScene();
			createInstanceGrid();
		}
	}
	else if (command == L"Scene.Editor.MoveDown")
	{
		if ((result = moveDown()) == true)
		{
			updateScene();
			createInstanceGrid();
		}
	}
	else if (command == L"Scene.Editor.NewLayer")
	{
		m_context->getDocument()->push();

		Ref< world::EntityData > layer = new world::EntityData();
		layer->setComponent(new world::GroupComponentData());

		auto layers = m_context->getSceneAsset()->getLayers();
		layers.push_back(layer);
		m_context->getSceneAsset()->setLayers(layers);

		updateScene();
		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.FilterEntity")
	{
		if (m_buttonFilterEntity->isToggled())
			m_entityFilterType = m_editor->browseType(
				makeTypeInfoSet< world::Entity, world::IEntityComponent >(),
				false,
				false
			);
		else
			m_entityFilterType = nullptr;

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.EnlargeGuide")
	{
		const float guideSize = m_context->getGuideSize();
		m_context->setGuideSize(guideSize + 0.5f);
		m_context->enqueueRedraw(nullptr);
	}
	else if (command == L"Scene.Editor.ShrinkGuide")
	{
		const float guideSize = m_context->getGuideSize();
		m_context->setGuideSize(std::max(guideSize - 0.5f, 0.5f));
		m_context->enqueueRedraw(nullptr);
	}
	else if (command == L"Scene.Editor.ResetGuide")
	{
		m_context->setGuideSize(1.0f);
		m_context->enqueueRedraw(nullptr);
	}
	else if (command == L"Scene.Editor.FindInDatabase")
	{
		const RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
		if (selectedEntities.size() != 1)
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
		for (auto selectedEntity : m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			selectedEntity->setLocked(true);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.UnlockEntities")
	{
		for (auto selectedEntity : m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			selectedEntity->setLocked(false);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.UnlockAllEntities")
	{
		for (auto selectedEntity : m_context->getEntities(SceneEditorContext::GfDescendants))
			selectedEntity->setLocked(false);

		createInstanceGrid();
	}
	else if (command == L"Scene.Editor.ShowEntities")
	{
		for (auto selectedEntity : m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			selectedEntity->setVisible(true);

		createInstanceGrid();
		m_context->enqueueRedraw(nullptr);
	}
	else if (command == L"Scene.Editor.ShowAllEntities")
	{
		for (auto entity : m_context->getEntities(SceneEditorContext::GfDescendants))
			entity->setVisible(true);

		createInstanceGrid();
		m_context->enqueueRedraw(nullptr);
	}
	else if (command == L"Scene.Editor.ShowOnlyEntities")
	{
		// Hide all entities, which are not lights etc.
		for (auto entity : m_context->getEntities(SceneEditorContext::GfDescendants))
		{
			if (entity->isGeometry())
				entity->setVisible(false);
		}

		// Show only selected entities.
		for (auto selectedEntity : m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
		{
			for (auto entity = selectedEntity; entity != nullptr; entity = entity->getParent())
				entity->setVisible(true);
		}

		createInstanceGrid();
		m_context->enqueueRedraw(nullptr);
	}
	else if (command == L"Scene.Editor.HideEntities")
	{
		for (auto selectedEntity : m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
			selectedEntity->setVisible(false);

		createInstanceGrid();
		m_context->enqueueRedraw(nullptr);
	}
	else if (command == L"Scene.Editor.RenameAllEntityIds")
	{
		if (ui::MessageBox::show(m_editControl, i18n::Text(L"SCENE_EDITOR_RENAME_ALL_ENTITY_IDS_MESSAGE"), i18n::Text(L"SCENE_EDITOR_RENAME_ALL_ENTITY_IDS_TITLE"), ui::MbIconExclamation | ui::MbYesNo) == ui::DialogResult::Yes)
		{
			SmallMap< Guid, Guid > renamedMap;

			// Create new IDs for each entity.
			for (auto entity : m_context->getEntities(SceneEditorContext::GfDescendants))
			{
				Guid newEntityId = Guid::create();
				if (entity->getEntityData()->getId().isNotNull())
					renamedMap.insert(entity->getEntityData()->getId(), newEntityId);
				entity->getEntityData()->setId(newEntityId);
			}

			// Also ensure attached data contain updated entity identities.
			for (auto operationData : m_context->getSceneAsset()->getOperationData())
				renameIds(operationData, renamedMap);
		}
	}
	else if (command == L"Scene.Editor.PlaceOnGround")
	{
		placeOnGround();
		m_context->enqueueRedraw(nullptr);
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

	bool externalModified = false;

	// Flush resource from manager.
	if (m_context->getResourceManager()->reload(eventId, false))
		externalModified = true;

	// Check if guid is used as an external reference.
	for (auto entityAdapter : m_context->getEntities())
	{
		Guid externalGuid;
		if (entityAdapter->getExternalGuid(externalGuid))
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

	if (auto sceneAsset = dynamic_type_cast< SceneAsset* >(documentObject))
		m_context->setSceneAsset(sceneAsset);
	else if (auto entityData = dynamic_type_cast< world::EntityData* >(documentObject))
	{
		Ref< SceneAsset > sceneAsset = m_context->getSourceDatabase()->getObjectReadOnly< SceneAsset >(c_guidWhiteRoomScene);
		if (!sceneAsset)
			return false;

		const auto& layers = sceneAsset->getLayers();
		T_ASSERT(layers.size() >= 2);

		layers[1]->getComponent< world::GroupComponentData >()->addEntityData(entityData);

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
		m_context->setControllerEditor(nullptr);
	}

	m_site->hideAdditionalPanel(m_controllerPanel);

	Ref< SceneAsset > sceneAsset = m_context->getSceneAsset();
	if (sceneAsset)
	{
		for (auto worldComponentData : sceneAsset->getWorldComponents())
		{
	 		RefArray< const IWorldComponentEditorFactory > componentEditorFactories;
	 		Ref< IWorldComponentEditor > componentEditor;

	 		// Create world component editor factories.
	 		for (auto profile : m_context->getEditorProfiles())
	 			profile->createControllerEditorFactories(m_context, componentEditorFactories);

	 		for (auto controllerEditorFactory : componentEditorFactories)
	 		{
	 			TypeInfoSet typeSet = controllerEditorFactory->getComponentDataTypes();
	 			if (typeSet.find(&type_of(worldComponentData)) != typeSet.end())
	 			{
	 				componentEditor = controllerEditorFactory->createComponentEditor(type_of(worldComponentData));
	 				if (componentEditor)
	 					break;
	 			}
	 		}

	 		if (componentEditor)
	 		{
	 			if (componentEditor->create(
	 				m_context,
	 				m_controllerPanel
	 			))
	 			{
	 				m_context->setControllerEditor(componentEditor);
	 				m_site->showAdditionalPanel(m_controllerPanel);
	 			}
	 			else
	 				log::error << L"Unable to create world component editor; create failed." << Endl;
	 		}
	 		else
	 			T_DEBUG(L"Unable to find world component editor for type \"" << type_name(worldComponentData) << L"\".");
	 	}
	}

	m_controllerPanel->update();
}

void SceneEditorPage::updateScene()
{
	m_context->buildEntities();
	m_context->enqueueRedraw(nullptr);
}

Ref< ui::GridRow > SceneEditorPage::createInstanceGridRow(EntityAdapter* entityAdapter)
{
	if (m_entityFilterType && !filterIncludeEntity(*m_entityFilterType, entityAdapter))
		return nullptr;

	// Assume root entities are layers.
	const bool layer = (bool)(entityAdapter->getParent() == nullptr);

	Ref< ui::GridRow > row = new ui::GridRow(0);
	row->setData(L"ENTITY", entityAdapter);
	row->setState(
		(entityAdapter->isSelected() ? ui::GridRow::Selected : 0) |
		(entityAdapter->isExpanded() ? ui::GridRow::Expanded : 0)
	);

	if (layer)
	{
		row->setMinimumHeight(32);
		row->setBackground(ui::ColorReference(this, L"background-color-layer"));
	}

	std::wstring entityName = entityAdapter->getName();
	if (entityName.empty())
		entityName = i18n::Text(L"SCENE_EDITOR_UNNAMED_ENTITY");

	// Create entity name item.
	Ref< ui::GridItem > item = new ui::GridItem(entityName);

	if (layer)
		item->setFont(m_instanceGridFontHuge);
	else if (entityAdapter->isExternal())
		item->setFont(m_instanceGridFontBold);

	if (entityAdapter->isGroup())
		item->addImage(new ui::StyleBitmap(L"Scene.EntityAttributeGroup"));
	if (entityAdapter->isPrefab())
		item->addImage(new ui::StyleBitmap(L"Scene.EntityAttributePrefab"));
	if (entityAdapter->isDynamic())
		item->addImage(new ui::StyleBitmap(L"Scene.EntityAttributeDynamic"));
	if (entityAdapter->isExternal())
		item->addImage(new ui::StyleBitmap(L"Scene.EntityAttributeExternal"));

	row->add(item);

	// Create "visible" check box.
	row->add(entityAdapter->isVisible(false) ? m_imageVisible : m_imageHidden);

	// Create "locked" check box.
	row->add(entityAdapter->isLocked(false) ? m_imageLocked : m_imageUnlocked);

	// Recursively add children.
	if (
		!entityAdapter->isExternal() &&
		!entityAdapter->isChildrenPrivate()
	)
	{
		for (auto child : entityAdapter->getChildren())
		{
			Ref< ui::GridRow > childRow = createInstanceGridRow(child);
			if (childRow)
				row->addChild(childRow);
		}
	}

	return row;
}

void SceneEditorPage::createInstanceGrid()
{
	m_instanceGrid->removeAllRows();
	for (auto layerEntityAdapter : m_context->getLayerEntityAdapters())
	{
		Ref< ui::GridRow > entityRow = createInstanceGridRow(layerEntityAdapter);
		if (entityRow)
			m_instanceGrid->addRow(entityRow);
	}
	m_instanceGrid->update();
}

void SceneEditorPage::updateInstanceGridRow(ui::GridRow* row)
{
	EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");

	row->setState(
		(entityAdapter->isSelected() ? ui::GridRow::Selected : 0) |
		(entityAdapter->isExpanded() ? ui::GridRow::Expanded : 0)
	);

	for (auto childRow : row->getChildren())
		updateInstanceGridRow(childRow);
}

void SceneEditorPage::updateInstanceGrid()
{
	for (auto row : m_instanceGrid->getRows())
		updateInstanceGridRow(row);

	m_instanceGrid->update();
}

void SceneEditorPage::updatePropertyObject()
{
	RefArray< EntityAdapter > entityAdapters = m_context->getEntities(SceneEditorContext::GfDescendants | SceneEditorContext::GfSelectedOnly);
	if (entityAdapters.size() == 1)
	{
		Ref< EntityAdapter > entityAdapter = entityAdapters.front();
		T_ASSERT(entityAdapter);

		m_propertiesView->setPropertyObject(entityAdapter->getEntityData());
	}
	else
		m_propertiesView->setPropertyObject(m_context->getDocument()->getObject(0));
}

void SceneEditorPage::updateStatusBar()
{
	const Camera* camera = m_context->getCamera(0);
	T_ASSERT(camera);

	Vector4 position = camera->getPosition();
	Vector4 angles = camera->getOrientation().toEulerAngles();

	m_statusBar->setText(0, str(L"%.2f, %.2f, %.2f", (float)position.x(), (float)position.y(), (float)position.z()));
	m_statusBar->setText(1, str(L"%.1f, %.1f, %.1f", rad2deg(angles.x()), rad2deg(angles.y()), rad2deg(angles.z())));
	m_statusBar->setText(2, str(L"%d entities", m_context->getEntityCount()));
	m_statusBar->setText(3, str(L"%.1f (%.1f)", m_context->getTime(), m_context->getTimeScale()));

	RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
	if (selectedEntities.size() == 1)
		m_statusBar->setText(4, selectedEntities[0]->getPath()/* + L" " + selectedEntities[0]->getEntityData()->getId().format()*/);
	else
		m_statusBar->setText(4, L"");
}

bool SceneEditorPage::addEntity(const TypeInfo* entityType)
{
	Ref< EntityAdapter > parentGroupAdapter;

	// Get selected entity, must be a single item.
	RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
	if (selectedEntities.size() == 1)
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
		if ((entityType = m_context->getEditor()->browseType(makeTypeInfoSet< world::EntityData >(), false, true)) == nullptr)
			return false;
	}

	Ref< world::EntityData > entityData = checked_type_cast< world::EntityData* >(entityType->createInstance());
	T_ASSERT(entityData);

	entityData->setId(Guid::create());

	m_context->getDocument()->push();

	Ref< EntityAdapter > entityAdapter = new EntityAdapter(m_context);
	entityAdapter->prepare(entityData, nullptr);
	parentGroupAdapter->addChild(nullptr, entityAdapter);

	// Select new entity.
	m_context->selectAllEntities(false);
	m_context->selectEntity(entityAdapter);

	updateScene();
	createInstanceGrid();
	updatePropertyObject();

	return true;
}

bool SceneEditorPage::addComponent()
{
	RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
	if (selectedEntities.size() != 1)
		return false;

	auto entityData = selectedEntities[0]->getEntityData();

	// Browse for component data type.
	const TypeInfo* componentType = m_context->getEditor()->browseType(makeTypeInfoSet< world::IEntityComponentData >(), false, true);
	if (componentType)
	{
		Ref< world::IEntityComponentData > componentData = dynamic_type_cast< world::IEntityComponentData* >(componentType->createInstance());
		if (componentData)
			entityData->setComponent(componentData);
	}
	else
		return false;

	updatePropertyObject();
	return true;
}

bool SceneEditorPage::createExternal()
{
	RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
	if (selectedEntities.size() != 1)
		return false;

	Ref< db::Group > group = m_editor->browseGroup();
	if (!group)
		return false;

	Ref< world::EntityData > entityData = selectedEntities[0]->getEntityData();
	T_FATAL_ASSERT(entityData != nullptr);
	
	std::wstring instanceName = entityData->getName();
	if (instanceName.empty())
		instanceName = L"Unnamed";

	Ref< db::Instance > instance = group->createInstance(instanceName);
	if (!instance)
		return false;

	instance->setObject(entityData);
	if (!instance->commit())
	{
		instance->revert();
		return false;
	}

	// \tbd Replace selected entity with external reference to entity.

	m_editor->updateDatabaseView();
	return true;
}

bool SceneEditorPage::resolveExternal()
{
	RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants | SceneEditorContext::GfExternalOnly);
	if (selectedEntities.size() != 1)
		return false;

	auto externalAdapter = selectedEntities.front();

	Guid externalId;
	if (!externalAdapter->getExternalGuid(externalId))
		return false;

	Ref< world::EntityData > resolvedEntityData = m_context->getSourceDatabase()->getObjectReadOnly< world::EntityData >(externalId);
	if (!resolvedEntityData)
	{
		log::error << L"Unable to resolve external; failed to read entity from database." << Endl;
		return false;
	}

	resolvedEntityData->setName(externalAdapter->getName());
	resolvedEntityData->setTransform(externalAdapter->getTransform0());

	auto parent = externalAdapter->getParent();
	T_FATAL_ASSERT(parent != nullptr);

	Ref< EntityAdapter > resolvedEntityAdapter = new EntityAdapter(m_context);
	resolvedEntityAdapter->prepare(resolvedEntityData, nullptr);

	parent->addChild(externalAdapter, resolvedEntityAdapter);
	parent->removeChild(externalAdapter);

	updateScene();
	createInstanceGrid();
	return true;
}

bool SceneEditorPage::moveToEntity()
{
	RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
	if (selectedEntities.size() == 1)
		m_context->moveToEntityAdapter(selectedEntities[0]);
	else
		m_context->moveToEntityAdapter(nullptr);
	return true;
}

bool SceneEditorPage::moveUp()
{
	RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
	if (selectedEntities.size() != 1)
		return false;

	EntityAdapter* moving = selectedEntities.back();
	T_ASSERT(moving != nullptr);

	EntityAdapter* parent = moving->getParent();
	if (!parent)
		return false;

	auto& children = parent->getChildren();
	auto it = std::find(children.begin(), children.end(), moving);
	if (it == children.begin() || it == children.end())
		return false;

	EntityAdapter* previousSibling = *(it - 1);
	T_ASSERT(previousSibling != nullptr);

	parent->swapChildren(moving, previousSibling);
	return true;
}

bool SceneEditorPage::moveDown()
{
	RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
	if (selectedEntities.size() != 1)
		return false;

	EntityAdapter* moving = selectedEntities.back();
	T_ASSERT(moving != nullptr);

	EntityAdapter* parent = moving->getParent();
	if (!parent)
		return false;

	auto& children = parent->getChildren();
	auto it = std::find(children.begin(), children.end(), moving);
	if (it == children.end() || it + 1 == children.end())
		return false;

	EntityAdapter* nextSibling = *(it + 1);
	T_ASSERT(nextSibling != nullptr);

	parent->swapChildren(moving, nextSibling);
	return true;
}

void SceneEditorPage::placeOnGround()
{
	const physics::QueryFilter filter;

	for (auto entity : m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants))
	{
		const Transform transform = entity->getTransform();
		const Vector4 position = transform.translation().xyz1();

		physics::QueryResult result;
		if (m_context->getPhysicsManager()->queryRay(
			position,
			Vector4(0.0f, -1.0f, 0.0f),
			1000.0f,
			filter,
			false,
			result
		))
		{
			const Quaternion Qrot(
				Vector4(0.0f, 1.0f, 0.0f),
				result.normal
			);

			entity->setTransform(Transform(
				result.position,
				Qrot // transform.rotation()
			));
		}
	}
}

void SceneEditorPage::eventEntityToolClick(ui::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void SceneEditorPage::eventGuideClick(ui::GridColumnClickEvent* event)
{
	if (event->getColumn() == 1)
	{
		ui::GridRow* row = event->getRow();
		const std::wstring id = row->get(0)->getText();

		const bool shouldDraw = !m_context->shouldDrawGuide(id);
		m_context->setDrawGuide(id, shouldDraw);

		row->set(1, new ui::GridItem(shouldDraw ? m_imageVisible : m_imageHidden));
		m_gridGuides->requestUpdate();

		m_editor->checkoutGlobalSettings()->setProperty< PropertyBoolean >(L"SceneEditor.Guides/" + id, shouldDraw);
		m_editor->commitGlobalSettings();

		m_context->enqueueRedraw(nullptr);
	}
}

void SceneEditorPage::eventInstanceSelect(ui::SelectionChangeEvent* event)
{
	// De-select all entities.
	m_context->selectAllEntities(false);

	// Select only entities which is selected in the grid.
	for (auto selectedRow : m_instanceGrid->getRows(ui::GridView::GfDescendants | ui::GridView::GfSelectedOnly))
	{
		EntityAdapter* entityAdapter = selectedRow->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT(entityAdapter);

		m_context->selectEntity(entityAdapter);
	}

	// Raise context select event and the ensure scene is redrawn.
	m_context->raiseSelect();
	m_context->enqueueRedraw(nullptr);
}

void SceneEditorPage::eventInstanceExpand(ui::GridRowStateChangeEvent* event)
{
	ui::GridRow* row = event->getRow();
	EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
	entityAdapter->setExpanded((row->getState() & ui::GridRow::Expanded) != 0);
}

void SceneEditorPage::eventInstanceButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() == ui::MbtRight)
	{
		const ui::MenuItem* selectedItem;

		RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
		if (selectedEntities.size() == 1)
		{
			if (selectedEntities[0]->isExternal())
				selectedItem = m_entityMenuExternal->showModal(m_instanceGrid, event->getPosition());
			else if (selectedEntities[0]->isGroup())
				selectedItem = m_entityMenuGroup->showModal(m_instanceGrid, event->getPosition());
			else
				selectedItem = m_entityMenuDefault->showModal(m_instanceGrid, event->getPosition());
		}

		if (selectedItem)
		{
			if (handleCommand(selectedItem->getCommand()))
				event->consume();
		}
	}
}

void SceneEditorPage::eventInstanceClick(ui::GridColumnClickEvent* event)
{
	if (event->getColumn() == 1)
	{
		ui::GridRow* row = event->getRow();
		ui::GridItem* item = row->get(1);

		EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT(entityAdapter);

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
		m_context->enqueueRedraw(nullptr);
	}
	else if (event->getColumn() == 2)
	{
		ui::GridRow* row = event->getRow();
		ui::GridItem* item = row->get(2);

		EntityAdapter* entityAdapter = row->getData< EntityAdapter >(L"ENTITY");
		T_ASSERT(entityAdapter);

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

void SceneEditorPage::eventInstanceRename(ui::GridItemContentChangeEvent* event)
{
	const std::wstring renameFrom = event->getOriginalText();
	const std::wstring renameTo = event->getItem()->getText();

	if (renameFrom == renameTo)
		return;

	ui::GridItem* item = event->getItem();
	if (item == nullptr || item->getRow() == nullptr)
		return;

	EntityAdapter* entityAdapter = item->getRow()->getData< EntityAdapter >(L"ENTITY");
	T_ASSERT(entityAdapter);

	m_context->getDocument()->push();

	entityAdapter->getEntityData()->setName(renameTo);
	updatePropertyObject();
	updateStatusBar();

	event->consume();
}

void SceneEditorPage::eventPropertiesChanging(ui::ContentChangingEvent* event)
{
	m_context->getDocument()->push();
}

void SceneEditorPage::eventPropertiesChanged(ui::ContentChangeEvent* event)
{
	RefArray< EntityAdapter > selectedEntities = m_context->getEntities(SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfDescendants);
	const ui::PropertyContentChangeEvent* propChange = dynamic_type_cast< const ui::PropertyContentChangeEvent* >(event);
	const bool trivialChange = (selectedEntities.size() == 1 && propChange != nullptr && is_a< ui::NumericPropertyItem >(propChange->getItem()));

	if (trivialChange)
	{
		updateScene();
	}
	else
	{
		updateScene();
		createInstanceGrid();

		// Notify controller editor as well.
		Ref< IWorldComponentEditor > controllerEditor = m_context->getControllerEditor();
		if (controllerEditor)
			controllerEditor->propertiesChanged();

		// Propagate to editor controls.
		m_editControl->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
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

void SceneEditorPage::eventContextPostFrame(PostFrameEvent* event)
{
	if (m_gridResources->isVisible(true))
	{
		m_gridResources->removeAllRows();

		render::RenderSystemStatistics rss;
		m_context->getRenderSystem()->getStatistics(rss);

		{
			Ref< ui::GridRow > row = new ui::GridRow();
			row->add(L"Buffers");
			row->add(str(L"%d", rss.buffers));
			m_gridResources->addRow(row);
		}
		{
			Ref< ui::GridRow > row = new ui::GridRow();
			row->add(L"Textures (2D)");
			row->add(str(L"%d", rss.simpleTextures));
			m_gridResources->addRow(row);
		}
		{
			Ref< ui::GridRow > row = new ui::GridRow();
			row->add(L"Textures (Cube)");
			row->add(str(L"%d", rss.cubeTextures));
			m_gridResources->addRow(row);
		}
		{
			Ref< ui::GridRow > row = new ui::GridRow();
			row->add(L"Textures (3D)");
			row->add(str(L"%d", rss.volumeTextures));
			m_gridResources->addRow(row);
		}
		{
			Ref< ui::GridRow > row = new ui::GridRow();
			row->add(L"Render target sets");
			row->add(str(L"%d", rss.renderTargetSets));
			m_gridResources->addRow(row);
		}
		{
			Ref< ui::GridRow > row = new ui::GridRow();
			row->add(L"Programs");
			row->add(str(L"%d", rss.programs));
			m_gridResources->addRow(row);
		}
	}

	updateStatusBar();
}

void SceneEditorPage::eventContextMeasurement(MeasurementEvent* event)
{
	if (!m_gridMeasurements->isVisible(true))
		return;

	if (event->getPass() <= 0)
		m_gridMeasurements->removeAllRows();

	auto& v = m_measurementVariance[event->getPass()];
	v.push_back(event->getDuration());

	double total = 0.0;
	for (uint32_t i = 0; i < v.size(); ++i)
		total += v[i];
	total /= (double)v.size();

	Ref< ui::GridRow > row = new ui::GridRow();
	row->add(str(L"%d [%d]", event->getPass(), event->getLevel()));
	row->add(event->getName());
	row->add(str(L"%d \xb5s", (int32_t)(total * 1000000.0)));
	m_gridMeasurements->addRow(row);
}

}
