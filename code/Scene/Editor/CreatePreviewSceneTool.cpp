/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Database/Group.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Scene/Editor/SceneAsset.h"
#include "Scene/Editor/CreatePreviewSceneTool.h"
#include "World/EntityData.h"
#include "World/Editor/LayerEntityData.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const Guid c_guidWhiteRoomScene(L"{B4AFD9C3-E47C-654B-99BE-B281AD10DD1A}");

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.CreatePreviewSceneTool", 0, CreatePreviewSceneTool, editor::IWizardTool)

std::wstring CreatePreviewSceneTool::getDescription() const
{
	return i18n::Text(L"SCENE_CREATE_PREVIEW_SCENE_DESCRIPTION");
}

uint32_t CreatePreviewSceneTool::getFlags() const
{
	return WfGroup;
}

bool CreatePreviewSceneTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	// Find all entities in selected group.
	RefArray< db::Instance > entityInstances;
	db::recursiveFindChildInstances(group, db::FindInstanceByType(type_of< world::EntityData >()), entityInstances);
	if (entityInstances.empty())
	{
		log::warning << L"No entity data instances found in selected group; no preview scene created." << Endl;
		return false;
	}

	// Read empty scene as template.
	Ref< SceneAsset > sceneAsset = editor->getSourceDatabase()->getObjectReadOnly< SceneAsset >(c_guidWhiteRoomScene);
	if (!sceneAsset)
	{
		log::error << L"Failed to create preview scene; unable to read scene template." << Endl;
		return false;
	}

	const RefArray< world::LayerEntityData >& layers = sceneAsset->getLayers();
	T_ASSERT (layers.size() >= 2);

	int32_t n = int32_t(std::sqrt(entityInstances.size()));
	if (n < 1)
		n = 1;

	for (int32_t i = 0; i < int32_t(entityInstances.size()); ++i)
	{
		Ref< world::EntityData > entityData = entityInstances[i]->getObject< world::EntityData >();
		if (!entityData)
		{
			log::error << L"Failed to create preview scene; unable to read entity \"" << entityInstances[i]->getPath() << L"\"." << Endl;
			return false;
		}

		int32_t x = i % n;
		int32_t z = i / n;

		entityData->setName(entityInstances[i]->getName());
		entityData->setTransform(Transform(
			Vector4(float((x - n / 2) * 32.0f), 0.0f, float((z - n / 2) * 32.0f), 0.0f)
		));
		layers[1]->addEntityData(entityData);
	}

	// Create output instance.
	Ref< db::Instance > sceneInstance = group->createInstance(
		L"Preview Scene",
		db::CifReplaceExisting | db::CifKeepExistingGuid
	);
	if (!sceneInstance)
		return false;

	sceneInstance->setObject(sceneAsset);

	if (!sceneInstance->commit())
		return false;

	return true;
}

	}
}
