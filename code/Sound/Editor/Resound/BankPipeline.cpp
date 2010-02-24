#include "Editor/IPipelineDepends.h"
#include "Sound/Resound/BankResource.h"
#include "Sound/Resound/PlayGrain.h"
#include "Sound/Resound/RepeatGrain.h"
#include "Sound/Resound/RandomGrain.h"
#include "Sound/Resound/SequenceGrain.h"
#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankPipeline.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

void buildGrainDependencies(editor::IPipelineDepends* pipelineDepends, const IGrain* grain)
{
	if (const RepeatGrain* repeatGrain = dynamic_type_cast< const RepeatGrain* >(grain))
		buildGrainDependencies(pipelineDepends, repeatGrain->getGrain());

	if (const RandomGrain* randomGrain = dynamic_type_cast< const RandomGrain* >(grain))
	{
		const RefArray< IGrain >& grains = randomGrain->getGrains();
		for (RefArray< IGrain >::const_iterator i = grains.begin(); i != grains.end(); ++i)
			buildGrainDependencies(pipelineDepends, *i);
	}

	if (const SequenceGrain* sequenceGrain = dynamic_type_cast< const SequenceGrain* >(grain))
	{
		const RefArray< IGrain >& grains = sequenceGrain->getGrains();
		for (RefArray< IGrain >::const_iterator i = grains.begin(); i != grains.end(); ++i)
			buildGrainDependencies(pipelineDepends, *i);
	}

	if (const PlayGrain* playGrain = dynamic_type_cast< const PlayGrain* >(grain))
		pipelineDepends->addDependency(playGrain->getSound().getGuid(), editor::PdfBuild);
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
	Ref< const Object >& outBuildParams
) const
{
	const BankAsset* bankAsset = checked_type_cast< const BankAsset* >(sourceAsset);
	const RefArray< IGrain >& grains = bankAsset->getGrains();
	for (RefArray< IGrain >::const_iterator i = grains.begin(); i != grains.end(); ++i)
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
