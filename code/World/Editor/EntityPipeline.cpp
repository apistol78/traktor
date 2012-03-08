#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "World/Editor/EntityDataTraverser.h"
#include "World/Editor/EntityPipeline.h"
#include "World/Entity/EntityData.h"
#include "World/Entity/ExternalEntityData.h"
#include "World/Entity/ExternalSpatialEntityData.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

class BuildDependenciesVisitor : public EntityDataTraverser::IVisitor
{
public:
	BuildDependenciesVisitor(editor::IPipelineDepends* pipelineDepends, const EntityData* currentEntityData)
	:	m_pipelineDepends(pipelineDepends)
	,	m_currentEntityData(currentEntityData)
	{
	}

	virtual EntityDataTraverser::VisitorResult enter (const std::wstring& memberName, EntityData* entityData, Ref< EntityData >& outEntityDataSubst)
	{
		m_pipelineDepends->addDependency(entityData);
		return EntityDataTraverser::VrSkip;
	}

	virtual EntityDataTraverser::VisitorResult leave (const std::wstring& memberName, EntityData* entityData, Ref< EntityData >& outEntityDataSubst)
	{
		return EntityDataTraverser::VrSkip;
	}

private:
	editor::IPipelineDepends* m_pipelineDepends;
	const EntityData* m_currentEntityData;
};

class BuildOutputVisitor : public EntityDataTraverser::IVisitor
{
public:
	BuildOutputVisitor(editor::IPipelineBuilder* pipelineBuilder)
	:	m_pipelineBuilder(pipelineBuilder)
	{
	}

	virtual EntityDataTraverser::VisitorResult enter (const std::wstring& memberName, EntityData* entityData, Ref< EntityData >& outEntityDataSubst)
	{
		outEntityDataSubst = checked_type_cast< EntityData* >(m_pipelineBuilder->buildOutput(entityData));
		return EntityDataTraverser::VrReplace;
	}

	virtual EntityDataTraverser::VisitorResult leave (const std::wstring& memberName, EntityData* entityData, Ref< EntityData >& outEntityDataSubst)
	{
		return EntityDataTraverser::VrSkip;
	}

private:
	editor::IPipelineBuilder* m_pipelineBuilder;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.EntityPipeline", 1, EntityPipeline, editor::IPipeline)

bool EntityPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void EntityPipeline::destroy()
{
}

TypeInfoSet EntityPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< EntityData >());
	return typeSet;
}

bool EntityPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const EntityData* entityData = checked_type_cast< const EntityData*, false >(sourceAsset);
	
	// Add dependencies from child entity data.
	EntityDataTraverser traverser(entityData);
	BuildDependenciesVisitor visitor(pipelineDepends, entityData);
	traverser.visit(visitor);

	// Add external entity data dependencies.
	if (const ExternalEntityData* externalEntityData = dynamic_type_cast< const ExternalEntityData* >(entityData))
		pipelineDepends->addDependency(externalEntityData->getGuid(), editor::PdfBuild);
	else if (const ExternalSpatialEntityData* externalSpatialEntityData = dynamic_type_cast< const ExternalSpatialEntityData* >(entityData))
		pipelineDepends->addDependency(externalSpatialEntityData->getGuid(), editor::PdfBuild);

	return true;
}

bool EntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	if ((reason & (editor::PbrSourceModified | editor::PbrForced)) == 0)
		return true;

	Ref< EntityData > entityData = checked_type_cast< EntityData*, true >(buildOutput(pipelineBuilder, sourceAsset));
	if (!entityData)
		return false;

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
		return false;

	outputInstance->setObject(entityData);

	if (!outputInstance->commit())
		return false;

	return true;
}

Ref< ISerializable > EntityPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	const EntityData* sourceEntityData = checked_type_cast< const EntityData*, false >(sourceAsset);
	
	// Create a mutable clone of source entity data.
	Ref< EntityData > entityData = clone_instance(sourceEntityData);
	if (!entityData)
	{
		log::error << L"Unable to clone entity data of type \"" << type_name(sourceEntityData) << L"\"" << Endl;
		return 0;
	}

	// Build child entity data of entity.
	EntityDataTraverser traverser(entityData);
	BuildOutputVisitor visitor(pipelineBuilder);
	traverser.visit(visitor);

	return entityData;
}

	}
}
