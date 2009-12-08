#include "Editor/IPipelineDepends.h"
#include "Sound/Resound/BankResource.h"
#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankPipeline.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankPipeline", 0, BankPipeline, editor::DefaultPipeline)

TypeInfoSet BankPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BankAsset >());
	return typeSet;
}

bool BankPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const BankAsset* bankAsset = checked_type_cast< const BankAsset* >(sourceAsset);

	const std::vector< resource::Proxy< Sound > >& sounds = bankAsset->getSounds();
	for (std::vector< resource::Proxy< Sound > >::const_iterator i = sounds.begin(); i != sounds.end(); ++i)
		pipelineDepends->addDependency(i->getGuid(), true);

	return true;
}

bool BankPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const BankAsset* bankAsset = checked_type_cast< const BankAsset* >(sourceAsset);

	Ref< BankResource > bankResource = new BankResource(
		bankAsset->getSounds()
	);

	return editor::DefaultPipeline::buildOutput(
		pipelineBuilder,
		bankResource,
		sourceAssetHash,
		buildParams,
		outputPath,
		outputGuid,
		reason
	);
}

	}
}
