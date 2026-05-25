/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializable.h"
#include "Database/Instance.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/MeshComponentRenderer.h"
#include "Mesh/MeshEntityFactory.h"
#include "Mesh/MeshResourceFactory.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshSceneEditorPlugin.h"
#include "Resource/Id.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "World/EntityData.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshSceneEditorPlugin", 0, MeshSceneEditorPlugin, scene::ISceneEditorPlugin)

void MeshSceneEditorPlugin::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void MeshSceneEditorPlugin::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
}

void MeshSceneEditorPlugin::createUIExtensions(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorUIExtension >& outUIExtensions
) const
{
}

void MeshSceneEditorPlugin::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new mesh::MeshResourceFactory(context->getRenderSystem()));
}

void MeshSceneEditorPlugin::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new mesh::MeshEntityFactory());
}

void MeshSceneEditorPlugin::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	const TypeInfo& worldRendererType,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new mesh::MeshComponentRenderer());
}

void MeshSceneEditorPlugin::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IWorldComponentEditorFactory >& outComponentEditorFactories) const
{
}

void MeshSceneEditorPlugin::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

void MeshSceneEditorPlugin::createComponentEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IComponentEditorFactory >& outComponentEditorFactories
) const
{
}

Ref< world::EntityData > MeshSceneEditorPlugin::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	const TypeInfo* primaryType = instance->getPrimaryType();
	if (!primaryType)
		return nullptr;

	if (!is_type_of< MeshAsset >(*primaryType))
		return nullptr;

	Ref< world::EntityData > entityData = new world::EntityData();
	entityData->setId(Guid::create());
	entityData->setName(instance->getName());
	entityData->setComponent(new MeshComponentData(
		resource::Id< IMesh >(instance->getGuid())
	));
	return entityData;
}

}
