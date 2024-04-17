/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/DeepClone.h"
#include "Editor/IEditor.h"
#include "Scene/Editor/ISceneEditorPlugin.h"
#include "Scene/Editor/ISceneEditorProfile.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/SceneEditorPage.h"
#include "Scene/Editor/SceneEditorPageFactory.h"
#include "Scene/Editor/Traverser.h"
#include "Ui/Command.h"
#include "World/EntityData.h"

namespace traktor::scene
{
	namespace
	{

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

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.SceneEditorPageFactory", 0, SceneEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet SceneEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< SceneAsset >();
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
	outDependencies.insert(Guid(L"{473467B0-835D-EF45-B308-E3C3C5B0F226}"));	// System/Scene/WhiteRoom
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
	outCommands.push_back(ui::Command(L"Scene.Editor.TogglePlay"));
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
	outCommands.push_back(ui::Command(L"Scene.Editor.SnapToNext"));
	outCommands.push_back(ui::Command(L"Scene.Editor.SnapToPrevious"));
	outCommands.push_back(ui::Command(L"Scene.Editor.FindInDatabase"));
	outCommands.push_back(ui::Command(L"Scene.Editor.LockEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.UnlockEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.UnlockAllEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ShowEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ShowAllEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.ShowOnlyEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.HideEntities"));
	outCommands.push_back(ui::Command(L"Scene.Editor.FilterEntity"));
	outCommands.push_back(ui::Command(L"Scene.Editor.RenameAllEntityIds"));
	outCommands.push_back(ui::Command(L"Scene.Editor.PlaceOnGround"));

	// Add profile commands.
	for (auto profileType : type_of< ISceneEditorProfile >().findAllOf())
	{
		Ref< ISceneEditorProfile > profile = dynamic_type_cast< ISceneEditorProfile* >(profileType->createInstance());
		if (profile)
			profile->getCommands(outCommands);
	}
}

Ref< ISerializable > SceneEditorPageFactory::cloneAsset(const ISerializable* asset) const
{
	Ref< ISerializable > mutableAsset = DeepClone(asset).create< ISerializable >();
	if (!mutableAsset)
		return nullptr;

	if (auto mutableSceneAsset = dynamic_type_cast< SceneAsset* >(mutableAsset))
	{
		SmallMap< Guid, Guid > renamedMap;

		// Rename all entities.
		Traverser::visit(mutableSceneAsset, [&](Ref< world::EntityData >& inoutEntityData) -> Traverser::Result {
			Guid newEntityId = Guid::create();
			if (inoutEntityData->getId().isNotNull())
				renamedMap.insert(inoutEntityData->getId(), newEntityId);
			inoutEntityData->setId(newEntityId);
			return Traverser::Result::Continue;
		});

		// Also ensure attached data contain updated entity identities.
		for (auto operationData : mutableSceneAsset->getOperationData())
			renameIds(operationData, renamedMap);
	}
	else if (auto mutableEntityData = dynamic_type_cast< world::EntityData* >(mutableAsset))
	{
		SmallMap< Guid, Guid > renamedMap;

		// Rename root entity data.
		Guid newEntityId = Guid::create();
		if (mutableEntityData->getId().isNotNull())
			renamedMap.insert(mutableEntityData->getId(), newEntityId);
		mutableEntityData->setId(newEntityId);

		// Rename all entities.
		Traverser::visit(mutableEntityData, [&](Ref< world::EntityData >& inoutEntityData) -> Traverser::Result {
			Guid newEntityId = Guid::create();
			if (inoutEntityData->getId().isNotNull())
				renamedMap.insert(inoutEntityData->getId(), newEntityId);
			inoutEntityData->setId(newEntityId);
			return Traverser::Result::Continue;
		});		
	}

	return mutableAsset;
}

}
