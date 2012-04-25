#include "Editor/IPipelineDepends.h"
#include "Sound/Resound/BankResource.h"
#include "Sound/Resound/PlayGrainData.h"
#include "Sound/Resound/RepeatGrainData.h"
#include "Sound/Resound/RandomGrainData.h"
#include "Sound/Resound/SequenceGrainData.h"
#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankPipeline.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

void buildGrainDependencies(editor::IPipelineDepends* pipelineDepends, const IGrainData* grain)
{
	if (const RepeatGrainData* repeatGrain = dynamic_type_cast< const RepeatGrainData* >(grain))
		buildGrainDependencies(pipelineDepends, repeatGrain->getGrain());

	if (const RandomGrainData* randomGrain = dynamic_type_cast< const RandomGrainData* >(grain))
	{
		const RefArray< IGrainData >& grains = randomGrain->getGrains();
		for (RefArray< IGrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
			buildGrainDependencies(pipelineDepends, *i);
	}

	if (const SequenceGrainData* sequenceGrain = dynamic_type_cast< const SequenceGrainData* >(grain))
	{
		const RefArray< IGrainData >& grains = sequenceGrain->getGrains();
		for (RefArray< IGrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
			buildGrainDependencies(pipelineDepends, *i);
	}

	if (const PlayGrainData* playGrain = dynamic_type_cast< const PlayGrainData* >(grain))
		pipelineDepends->addDependency(playGrain->getSound(), editor::PdfBuild);
}

		}

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
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	const BankAsset* bankAsset = checked_type_cast< const BankAsset* >(sourceAsset);
	const RefArray< IGrainData >& grains = bankAsset->getGrains();
	for (RefArray< IGrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
		buildGrainDependencies(pipelineDepends, *i);
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
		bankAsset->getGrains()
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
