#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Mesh/MeshEntityData.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/Batch/BatchMeshEntityData.h"
#include "Mesh/Editor/Batch/BatchMeshEntityPipeline.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/MergeModel.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

template < typename PipelineType >
Ref< ISerializable > resolveAllExternal(PipelineType* pipeline, const ISerializable* object)
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
				return 0;

			Ref< ISerializable > resolvedEntityData = resolveAllExternal(pipeline, externalEntityData);
			if (!resolvedEntityData)
				return 0;

			objectMember->set(resolvedEntityData);
		}
		else if (objectMember->get())
		{
			objectMember->set(resolveAllExternal(pipeline, objectMember->get()));
		}
	}

	return reflection->clone();
}

void findAllMeshEntityData(const ISerializable* object, RefArray< MeshEntityData >& outMeshEntityData)
{
	Ref< Reflection > reflection = Reflection::create(object);

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (MeshEntityData* meshEntityData = dynamic_type_cast< MeshEntityData* >(objectMember->get()))
		{
			outMeshEntityData.push_back(meshEntityData);
		}
		else if (objectMember->get())
		{
			findAllMeshEntityData(objectMember->get(), outMeshEntityData);
		}
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.BatchMeshEntityPipeline", 0, BatchMeshEntityPipeline, world::EntityPipeline)

bool BatchMeshEntityPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

TypeInfoSet BatchMeshEntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BatchMeshEntityData >());
	return typeSet;
}

bool BatchMeshEntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	Ref< BatchMeshEntityData > batchMeshEntityData = checked_type_cast< BatchMeshEntityData* >(resolveAllExternal(pipelineDepends, sourceAsset));

	// \fixme Add dependencies.

	return true;
}

Ref< ISerializable > BatchMeshEntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	Ref< BatchMeshEntityData > batchMeshEntityData = checked_type_cast< BatchMeshEntityData* >(resolveAllExternal(pipelineBuilder, sourceAsset));

	// Get all mesh entities.
	RefArray< MeshEntityData > meshEntityData;
	findAllMeshEntityData(batchMeshEntityData, meshEntityData);

	// Merge all models into a single big model.
	Ref< model::Model > mergedModel = new model::Model();
	std::map< std::wstring, Guid > mergedMaterialShaders;
	std::map< std::wstring, Guid > mergedMaterialTextures;

	for (RefArray< MeshEntityData >::const_iterator i = meshEntityData.begin(); i != meshEntityData.end(); ++i)
	{
		Guid meshAssetGuid = (*i)->getMesh();

		Ref< const MeshAsset > meshAsset = pipelineBuilder->getObjectReadOnly< MeshAsset >(meshAssetGuid);
		if (!meshAsset)
		{
			log::warning << L"Unable to read mesh asset \"" << meshAssetGuid.format() << L"\"" << Endl;
			continue;
		}

		// Insert custom material shaders.
		const std::map< std::wstring, Guid >& materialShaders = meshAsset->getMaterialShaders();
		for (std::map< std::wstring, Guid >::const_iterator j = materialShaders.begin(); j != materialShaders.end(); ++j)
			mergedMaterialShaders[j->first] = j->second;

		// Insert material textures.
		const std::map< std::wstring, Guid >& materialTextures = meshAsset->getMaterialTextures();
		for (std::map< std::wstring, Guid >::const_iterator j = materialTextures.begin(); j != materialTextures.end(); ++j)
			mergedMaterialTextures[j->first] = j->second;

		Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, meshAsset->getFileName());

		Ref< model::Model > partModel = model::ModelFormat::readAny(fileName);
		if (!partModel)
		{
			log::warning << L"Unable to read model \"" << fileName.getPathName() << L"\"" << Endl;
			continue;
		}

		model::MergeModel(*partModel, (*i)->getTransform(), 0.01f).apply(*mergedModel);
	}

	// Build output mesh from merged model.
	Ref< MeshAsset > mergedMeshAsset = new MeshAsset();
	mergedMeshAsset->setMeshType(MeshAsset::MtStatic);
	mergedMeshAsset->setMaterialShaders(mergedMaterialShaders);
	mergedMeshAsset->setMaterialTextures(mergedMaterialTextures);

	pipelineBuilder->buildOutput(
		mergedMeshAsset,
		L"Generated/" + batchMeshEntityData->getOutputGuid().format(),
		batchMeshEntityData->getOutputGuid(),
		mergedModel
	);

	// Create replacement mesh entity.
	Ref< MeshEntityData > outputEntityData = new MeshEntityData();
	outputEntityData->setName(batchMeshEntityData->getName());
	outputEntityData->setTransform(batchMeshEntityData->getTransform());
	outputEntityData->setMesh(resource::Id< IMesh >(batchMeshEntityData->getOutputGuid()));

	return outputEntityData;
}

	}
}
