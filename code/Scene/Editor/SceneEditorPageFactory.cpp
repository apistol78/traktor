#include "Scene/Editor/SceneEditorPageFactory.h"
#include "Scene/Editor/SceneEditorPage.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/ISceneEditorPlugin.h"
#include "Scene/SceneAsset.h"
#include "Editor/IEditor.h"
#include "Editor/IProject.h"
#include "Editor/Settings.h"
#include "Resource/ResourceManager.h"
#include "Physics/PhysicsManager.h"
#include "World/Entity/EntityData.h"
#include "Ui/Command.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneEditorPageFactory", 0, SceneEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet SceneEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SceneAsset >());
	typeSet.insert(&type_of< world::EntityData >());
	return typeSet;
}

Ref< editor::IEditorPage > SceneEditorPageFactory::createEditorPage(editor::IEditor* editor) const
{
	Ref< editor::IProject > project = editor->getProject();
	T_ASSERT (project);

	if (!editor->getRenderSystem())
	{
		log::error << L"Unable to create scene editor; render system required." << Endl;
		return 0;
	}

	// Create resource manager.
	Ref< resource::IResourceManager > resourceManager = new resource::ResourceManager();

	// Get physics manager type.
	std::wstring physicsManagerTypeName = editor->getSettings()->getProperty< editor::PropertyString >(L"SceneEditor.PhysicsManager");
	const TypeInfo* physicsManagerType = TypeInfo::find(physicsManagerTypeName);
	if (!physicsManagerType)
	{
		log::error << L"Unable to create scene editor; no such physics manager type \"" << physicsManagerTypeName << L"\"." << Endl;
		return 0;
	}

	// Create physics manager.
	Ref< physics::PhysicsManager > physicsManager = checked_type_cast< physics::PhysicsManager* >(physicsManagerType->createInstance());
	if (!physicsManager->create(1.0f / 60.0f))
	{
		log::error << L"Unable to create scene editor; failed to create physics manager." << Endl;
		return 0;
	}

	// Configure physics manager.
	physicsManager->setGravity(Vector4(0.0f, -9.81f, 0.0f, 0.0f));

	log::debug << L"Using physics manager \"" << physicsManagerTypeName << L"\"; created successfully" << Endl;

	// Create editor context.
	Ref< SceneEditorContext > context = new SceneEditorContext(
		editor,
		project->getOutputDatabase(),
		project->getSourceDatabase(),
		resourceManager,
		editor->getRenderSystem(),
		physicsManager
	);

	// Create profiles, plugins, resource factories and entity editors.
	std::vector< const TypeInfo* > profileTypes;
	type_of< ISceneEditorProfile >().findAllOf(profileTypes);
	for (std::vector< const TypeInfo* >::const_iterator i = profileTypes.begin(); i != profileTypes.end(); ++i)
	{
		Ref< ISceneEditorProfile > profile = dynamic_type_cast< ISceneEditorProfile* >((*i)->createInstance());
		if (!profile)
			continue;

		context->addEditorProfile(profile);

		RefArray< ISceneEditorPlugin > editorPlugins;
		profile->createEditorPlugins(context, editorPlugins);
		for (RefArray< ISceneEditorPlugin >::iterator j = editorPlugins.begin(); j != editorPlugins.end(); ++j)
			context->addEditorPlugin(*j);

		RefArray< resource::IResourceFactory > resourceFactories;
		profile->createResourceFactories(context, resourceFactories);
		for (RefArray< resource::IResourceFactory >::iterator j = resourceFactories.begin(); j != resourceFactories.end(); ++j)
			resourceManager->addFactory(*j);

	}

	// Create editor page.
	return new SceneEditorPage(context);
}

void SceneEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	// Add editor commands.
	outCommands.push_back(ui::Command(L"Scene.Editor.AddEntity"));
	outCommands.push_back(ui::Command(L"Scene.Editor.Translate"));
	outCommands.push_back(ui::Command(L"Scene.Editor.Rotate"));
	outCommands.push_back(ui::Command(L"Scene.Editor.TogglePick"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ToggleX"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ToggleY"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ToggleZ"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ToggleGrid"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ToggleGuide"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ToggleSnap"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ToggleAddReference"));
	outCommands.push_back(ui::Command(L"Scene.Editor.Rewind"));
	outCommands.push_back(ui::Command(L"Scene.Editor.Play"));
	outCommands.push_back(ui::Command(L"Scene.Editor.Stop"));
	outCommands.push_back(ui::Command(L"Scene.Editor.SingleView"));
	outCommands.push_back(ui::Command(L"Scene.Editor.DoubleView"));
	outCommands.push_back(ui::Command(L"Scene.Editor.QuadrupleView"));

	// Add profile commands.
	std::vector< const TypeInfo* > profileTypes;
	type_of< ISceneEditorProfile >().findAllOf(profileTypes);
	for (std::vector< const TypeInfo* >::const_iterator i = profileTypes.begin(); i != profileTypes.end(); ++i)
	{
		Ref< ISceneEditorProfile > profile = dynamic_type_cast< ISceneEditorProfile* >((*i)->createInstance());
		if (profile)
			profile->getCommands(outCommands);
	}
}

	}
}
