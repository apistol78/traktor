#include "Core/Serialization/ISerializable.h"
#include "Database/Instance.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/MeshEntityFactory.h"
#include "Mesh/MeshComponentRenderer.h"
#include "Mesh/MeshEntityRenderer.h"
#include "Mesh/MeshFactory.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshEditorProfile.h"
#include "Mesh/Editor/Batch/BatchMeshEntityFactory.h"
#include "Mesh/Instance/InstanceMeshComponentRenderer.h"
#include "Mesh/Instance/InstanceMeshEntityRenderer.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.MeshEditorProfile", 0, MeshEditorProfile, scene::ISceneEditorProfile)

void MeshEditorProfile::getCommands(
	std::list< ui::Command >& outCommands
) const
{
}

void MeshEditorProfile::getGuideDrawIds(
	std::set< std::wstring >& outIds
) const
{
}

void MeshEditorProfile::createEditorPlugins(
	scene::SceneEditorContext* context,
	RefArray< scene::ISceneEditorPlugin >& outEditorPlugins
) const
{
}

void MeshEditorProfile::createResourceFactories(
	scene::SceneEditorContext* context,
	RefArray< const resource::IResourceFactory >& outResourceFactories
) const
{
	outResourceFactories.push_back(new mesh::MeshFactory(context->getRenderSystem()));
}

void MeshEditorProfile::createEntityFactories(
	scene::SceneEditorContext* context,
	RefArray< const world::IEntityFactory >& outEntityFactories
) const
{
	outEntityFactories.push_back(new mesh::MeshEntityFactory(context->getResourceManager()));
	outEntityFactories.push_back(new mesh::BatchMeshEntityFactory(context->getResourceManager()));
}

void MeshEditorProfile::createEntityRenderers(
	scene::SceneEditorContext* context,
	render::IRenderView* renderView,
	render::PrimitiveRenderer* primitiveRenderer,
	RefArray< world::IEntityRenderer >& outEntityRenderers
) const
{
	outEntityRenderers.push_back(new mesh::MeshComponentRenderer());
	outEntityRenderers.push_back(new mesh::MeshEntityRenderer());
	outEntityRenderers.push_back(new mesh::InstanceMeshComponentRenderer());
	outEntityRenderers.push_back(new mesh::InstanceMeshEntityRenderer());
}

void MeshEditorProfile::createControllerEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::ISceneControllerEditorFactory >& outControllerEditorFactories
) const
{
}

void MeshEditorProfile::createEntityEditorFactories(
	scene::SceneEditorContext* context,
	RefArray< const scene::IEntityEditorFactory >& outEntityEditorFactories
) const
{
}

Ref< world::EntityData > MeshEditorProfile::createEntityData(
	scene::SceneEditorContext* context,
	db::Instance* instance
) const
{
	const TypeInfo* primaryType = instance->getPrimaryType();
	if (!primaryType)
		return 0;

	if (!is_type_of< MeshAsset >(*primaryType))
		return 0;

	Ref< MeshEntityData > entityData = new MeshEntityData();
	entityData->setName(instance->getName());
	entityData->setMesh(resource::Id< IMesh >(instance->getGuid()));

	return entityData;
}

	}
}
