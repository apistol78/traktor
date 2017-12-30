#include "Amalgam/Editor/Prefab/PrefabEntityData.h"
#include "Amalgam/Editor/Prefab/PrefabEntityPipeline.h"
#include "Amalgam/Editor/Prefab/PrefabMerge.h"
#include "Core/Log/Log.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "World/Entity/ComponentEntityData.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

Ref< ISerializable > resolveAllExternal(editor::IPipelineCommon* pipeline, const ISerializable* object)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (const world::ExternalEntityData* externalEntityDataRef = dynamic_type_cast< const world::ExternalEntityData* >(objectMember->get()))
		{
			Ref< const ISerializable > externalEntityData = pipeline->getObjectReadOnly(externalEntityDataRef->getEntityData());
			if (!externalEntityData)
			{
				log::error << L"PrefabEntityPipeline failed; Unable to read external entity." << Endl;
				return 0;
			}

			Ref< world::EntityData > resolvedEntityData = dynamic_type_cast< world::EntityData* >(resolveAllExternal(pipeline, externalEntityData));
			if (!resolvedEntityData)
			{
				log::error << L"PrefabEntityPipeline failed; Unable to resolve external entity." << Endl;
				return 0;
			}

			resolvedEntityData->setName(externalEntityDataRef->getName());
			resolvedEntityData->setTransform(externalEntityDataRef->getTransform());

			objectMember->set(resolvedEntityData);
		}
		else if (objectMember->get())
		{
			objectMember->set(resolveAllExternal(pipeline, objectMember->get()));
		}
	}

	return reflection->clone();
}

void collectComponentEntities(const ISerializable* object, RefArray< world::ComponentEntityData >& outComponentEntities)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (world::ComponentEntityData* componentEntityData = dynamic_type_cast< world::ComponentEntityData* >(objectMember->get()))
		{
			outComponentEntities.push_back(componentEntityData);
		}
		else if (objectMember->get())
		{
			collectComponentEntities(objectMember->get(), outComponentEntities);
		}
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.PrefabEntityPipeline", 0, PrefabEntityPipeline, world::EntityPipeline)

PrefabEntityPipeline::PrefabEntityPipeline()
:	m_targetEditor(false)
{
}

bool PrefabEntityPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	m_targetEditor = settings->getProperty< bool >(L"Pipeline.TargetEditor", false);
	m_usedGuids.clear();
	return true;
}

TypeInfoSet PrefabEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PrefabEntityData >());
	return typeSet;
}

bool PrefabEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const PrefabEntityData* sourcePrefabEntityData = checked_type_cast< const PrefabEntityData* >(sourceAsset);

	// Verify integrity of output guids.
	if (m_usedGuids.find(sourcePrefabEntityData->getOutputGuid(0)) != m_usedGuids.end())
	{
		log::error << L"PrefabEntityPipeline failed; Output guid 0 of prefab \"" << sourcePrefabEntityData->getName() << L"\" already used." << Endl;
		return 0;
	}

	m_usedGuids.insert(sourcePrefabEntityData->getOutputGuid(0));

	if (m_usedGuids.find(sourcePrefabEntityData->getOutputGuid(1)) != m_usedGuids.end())
	{
		log::error << L"PrefabEntityPipeline failed; Output guid 1 of prefab \"" << sourcePrefabEntityData->getName() << L"\" already used." << Endl;
		return 0;
	}

	m_usedGuids.insert(sourcePrefabEntityData->getOutputGuid(1));

	if (!m_targetEditor)
	{
		Ref< PrefabEntityData > prefabEntityData = checked_type_cast< PrefabEntityData* >(resolveAllExternal(pipelineDepends, sourcePrefabEntityData));
		if (!prefabEntityData)
		{
			log::error << L"PrefabEntityPipeline failed; unable to resolve all external entities" << Endl;
			return false;
		}

		Guid outputRenderMeshGuid = prefabEntityData->getOutputGuid(0);
		Guid outputCollisionShapeGuid = prefabEntityData->getOutputGuid(1);

		Transform Tprefab = prefabEntityData->getTransform();
		Transform TprefabInv = Tprefab.inverse();

		Ref< PrefabMerge > prefabMergeRender = new PrefabMerge(prefabEntityData->partitionMesh());
		prefabMergeRender->setName(prefabEntityData->getName() + L"_Render");

		Ref< PrefabMerge > prefabMergeCollision = new PrefabMerge();
		prefabMergeCollision->setName(prefabEntityData->getName() + L"_Collision");

		// Get all component entities.
		RefArray< world::ComponentEntityData > componentEntityData;
		collectComponentEntities(prefabEntityData, componentEntityData);

		for (RefArray< world::ComponentEntityData >::const_iterator i = componentEntityData.begin(); i != componentEntityData.end(); ++i)
		{
			const mesh::MeshComponentData* meshComponent = (*i)->getComponent< mesh::MeshComponentData >();
			if (meshComponent)
			{
				Guid meshAssetGuid = meshComponent->getMesh();
				Ref< const mesh::MeshAsset > meshAsset = pipelineDepends->getObjectReadOnly< mesh::MeshAsset >(meshAssetGuid);
				if (!meshAsset)
				{
					log::warning << L"Skipped entity \"" << (*i)->getName() << L"\"; unable to read visual mesh asset" << Endl;
					continue;
				}

				prefabMergeRender->addVisualMesh(meshAsset, TprefabInv * (*i)->getTransform());
			}

			const physics::RigidBodyComponentData* rigidBodyComponent = (*i)->getComponent< physics::RigidBodyComponentData >();
			if (rigidBodyComponent)
			{
				const physics::BodyDesc* bodyDesc = rigidBodyComponent->getBodyDesc();
				if (bodyDesc)
				{
					const physics::MeshShapeDesc* meshShapeDesc = dynamic_type_cast< const physics::MeshShapeDesc* >(bodyDesc->getShape());
					if (meshShapeDesc)
					{
						Guid meshShapeAssetGuid = meshShapeDesc->getMesh();
						Ref< const physics::MeshAsset > meshShapeAsset = pipelineDepends->getObjectReadOnly< physics::MeshAsset >(meshShapeAssetGuid);
						if (!meshShapeAsset)
						{
							log::warning << L"Skipped entity \"" << (*i)->getName() << L"\"; unable to read collision mesh asset" << Endl;
							continue;
						}

						prefabMergeCollision->addShapeMesh(meshShapeAsset, TprefabInv * (*i)->getTransform());
					}
					else
						log::warning << L"Prefab \"" << prefabEntityData->getName() << L"\"'s entity \"" << (*i)->getName() << L"\" must have a mesh collision shape; " << type_name(bodyDesc->getShape()) << L" not supported in prefabs" << Endl;
				}
			}
		}

		if (!prefabMergeRender->getVisualMeshes().empty())
			pipelineDepends->addDependency(
				prefabMergeRender,
				L"Generated/Mesh_" + outputRenderMeshGuid.format(),
				outputRenderMeshGuid,
				editor::PdfBuild | editor::PdfResource
			);

		if (!prefabMergeCollision->getShapeMeshes().empty())
			pipelineDepends->addDependency(
				prefabMergeCollision,
				L"Generated/Collision_" + outputCollisionShapeGuid.format(),
				outputCollisionShapeGuid,
				editor::PdfBuild | editor::PdfResource
			);

		return true;
	}
	else
		return world::EntityPipeline::buildDependencies(
			pipelineDepends,
			sourceInstance,
			sourceAsset,
			outputPath,
			outputGuid
		);
}

Ref< ISerializable > PrefabEntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	if (!m_targetEditor)
	{
		Ref< PrefabEntityData > prefabEntityData = checked_type_cast< PrefabEntityData* >(resolveAllExternal(pipelineBuilder, sourceAsset));
		if (!prefabEntityData)
		{
			log::error << L"Prefab entity pipeline failed; Unable to resolve all external entities" << Endl;
			return 0;
		}

		Transform Tprefab = prefabEntityData->getTransform();

		Guid outputRenderMeshGuid = prefabEntityData->getOutputGuid(0);
		Guid outputCollisionShapeGuid = prefabEntityData->getOutputGuid(1);

		// Get all rigid entities.
		RefArray< world::ComponentEntityData > componentEntityData;
		collectComponentEntities(prefabEntityData, componentEntityData);

		uint32_t renderMeshes = 0;
		uint32_t shapeMeshes = 0;
		std::set< resource::Id< physics::CollisionSpecification > > shapeCollisionGroup;
		std::set< resource::Id< physics::CollisionSpecification > > shapeCollisionMask;

		for (RefArray< world::ComponentEntityData >::const_iterator i = componentEntityData.begin(); i != componentEntityData.end(); ++i)
		{
			const mesh::MeshComponentData* meshComponent = (*i)->getComponent< mesh::MeshComponentData >();
			if (meshComponent)
				++renderMeshes;

			const physics::RigidBodyComponentData* rigidBodyComponent = (*i)->getComponent< physics::RigidBodyComponentData >();
			if (rigidBodyComponent)
			{
				const physics::BodyDesc* bodyDesc = rigidBodyComponent->getBodyDesc();
				if (bodyDesc)
				{
					const physics::MeshShapeDesc* meshShapeDesc = dynamic_type_cast< const physics::MeshShapeDesc* >(bodyDesc->getShape());
					if (meshShapeDesc)
					{
						++shapeMeshes;
						shapeCollisionGroup.insert(meshShapeDesc->getCollisionGroup().begin(), meshShapeDesc->getCollisionGroup().end());
						shapeCollisionMask.insert(meshShapeDesc->getCollisionMask().begin(), meshShapeDesc->getCollisionMask().end());
					}
				}
			}
		}

		Ref< world::ComponentEntityData > outputEntityData = new world::ComponentEntityData();
		outputEntityData->setName(prefabEntityData->getName());
		outputEntityData->setTransform(Tprefab);

		if (shapeMeshes > 0 && outputCollisionShapeGuid.isNotNull())
		{
			Ref< physics::MeshShapeDesc > outputShapeDesc = new physics::MeshShapeDesc();
			outputShapeDesc->setMesh(resource::Id< physics::Mesh >(outputCollisionShapeGuid));
			outputShapeDesc->setCollisionGroup(shapeCollisionGroup);
			outputShapeDesc->setCollisionMask(shapeCollisionMask);

			Ref< physics::StaticBodyDesc > outputBodyDesc = new physics::StaticBodyDesc();
			outputBodyDesc->setShape(outputShapeDesc);

			Ref< physics::RigidBodyComponentData > outputRigidBodyComponent = new physics::RigidBodyComponentData(
				outputBodyDesc
			);
			outputEntityData->setComponent(outputRigidBodyComponent);
		}

		if (renderMeshes > 0 && outputRenderMeshGuid.isNotNull())
		{
			Ref< mesh::MeshComponentData > outputMeshComponent = new mesh::MeshComponentData(
				resource::Id< mesh::IMesh >(outputRenderMeshGuid)
			);
			outputEntityData->setComponent(outputMeshComponent);
		}

		return outputEntityData;
	}
	else
		return world::EntityPipeline::buildOutput(
			pipelineBuilder,
			sourceAsset
		);
}

	}
}
