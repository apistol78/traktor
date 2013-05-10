#include "Core/Log/Log.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Serialization/ISerializable.h"
#include "Database/Instance.h"
#include "Editor/DefaultPipeline.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.DefaultPipeline", 2, DefaultPipeline, IPipeline)

bool DefaultPipeline::create(const IPipelineSettings* settings)
{
	return true;
}

void DefaultPipeline::destroy()
{
}

TypeInfoSet DefaultPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Object >());
	return typeSet;
}

bool DefaultPipeline::buildDependencies(
	IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{

	Ref< Reflection > reflection = Reflection::create(sourceAsset);
	if (!reflection)
		return false;

	RefArray< ReflectionMember > objectMembers;
	reflection->findMembers(RfpMemberType(type_of< RfmObject >()), objectMembers);

	while (!objectMembers.empty())
	{
		Ref< const RfmObject > objectMember = checked_type_cast< RfmObject*, false >(objectMembers.front());
		objectMembers.pop_front();

		if (objectMember->get() != 0 && objectMember->get() != sourceAsset)
			pipelineDepends->addDependency(objectMember->get());
	}

	return true;
}

bool DefaultPipeline::buildOutput(
	IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(sourceAsset);

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > DefaultPipeline::buildOutput(
	IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
