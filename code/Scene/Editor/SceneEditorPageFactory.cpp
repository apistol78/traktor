#include "Core/Log/Log.h"
#include "Editor/IEditor.h"
#include "Scene/Editor/ISceneEditorPlugin.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorPage.h"
#include "Scene/Editor/SceneEditorPageFactory.h"
#include "Ui/Command.h"
#include "World/EntityData.h"

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

bool SceneEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}"));	// System/Primitive/Shaders/Primitive
	outDependencies.insert(Guid(L"{123602E4-BC6F-874D-92E8-A20852D140A3}"));	// System/Primitive/Textures/SmallFont
	outDependencies.insert(Guid(L"{4D4647F5-AC2D-B04A-AAC9-309A7BC9D980}"));	// System/World/WorldDeferredAssets
	outDependencies.insert(Guid(L"{BCCEE4A9-4054-5A46-B179-8353133C0D65}"));	// System/World/WorldForwardAssets
	outDependencies.insert(Guid(L"{949B3C96-0196-F24E-B36E-98DD504BCE9D}"));	// System/Scene/Shaders/DebugTargets
	return true;
}

Ref< editor::IEditorPage > SceneEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new SceneEditorPage(editor, site, document);
}

void SceneEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	// Add editor commands.
	outCommands.push_back(ui::Command(L"Scene.Editor.AddEntity"));
	outCommands.push_back(ui::Command(L"Scene.Editor.Translate"));
	outCommands.push_back(ui::Command(L"Scene.Editor.Rotate"));
	outCommands.push_back(ui::Command(L"Scene.Editor.TogglePick"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ToggleGrid"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ToggleGuide"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ToggleSnap"));
	outCommands.push_back(ui::Command(L"Scene.Editor.IncreaseSnap"));
	outCommands.push_back(ui::Command(L"Scene.Editor.DecreaseSnap"));
	outCommands.push_back(ui::Command(L"Scene.Editor.MoveToEntity"));
	outCommands.push_back(ui::Command(L"Scene.Editor.Rewind"));
	outCommands.push_back(ui::Command(L"Scene.Editor.Play"));
	outCommands.push_back(ui::Command(L"Scene.Editor.Stop"));
	outCommands.push_back(ui::Command(L"Scene.Editor.SingleView"));
	outCommands.push_back(ui::Command(L"Scene.Editor.DoubleView"));
	outCommands.push_back(ui::Command(L"Scene.Editor.QuadrupleView"));
	outCommands.push_back(ui::Command(L"Scene.Editor.EnlargeGuide"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ShrinkGuide"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ResetGuide"));
	outCommands.push_back(ui::Command(L"Scene.Editor.Snap"));
	outCommands.push_back(ui::Command(L"Scene.Editor.RotatePlus30"));
	outCommands.push_back(ui::Command(L"Scene.Editor.RotatePlus45"));
	outCommands.push_back(ui::Command(L"Scene.Editor.RotatePlus90"));
	outCommands.push_back(ui::Command(L"Scene.Editor.RotateMinus30"));
	outCommands.push_back(ui::Command(L"Scene.Editor.RotateMinus45"));
	outCommands.push_back(ui::Command(L"Scene.Editor.RotateMinus90"));
	outCommands.push_back(ui::Command(L"Scene.Editor.FindInDatabase"));
	outCommands.push_back(ui::Command(L"Scene.Editor.LockEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.UnlockEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.UnlockAllEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ShowEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ShowAllEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.HideEntities"));

	// Add profile commands.
	TypeInfoSet profileTypes;
	type_of< ISceneEditorProfile >().findAllOf(profileTypes);
	for (TypeInfoSet::const_iterator i = profileTypes.begin(); i != profileTypes.end(); ++i)
	{
		Ref< ISceneEditorProfile > profile = dynamic_type_cast< ISceneEditorProfile* >((*i)->createInstance());
		if (profile)
			profile->getCommands(outCommands);
	}
}

	}
}
