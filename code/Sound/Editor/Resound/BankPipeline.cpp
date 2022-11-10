/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/MathUtils.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Sound/Resound/BankResource.h"
#include "Sound/Resound/BlendGrainData.h"
#include "Sound/Resound/EnvelopeGrainData.h"
#include "Sound/Resound/InLoopOutGrainData.h"
#include "Sound/Resound/PlayGrainData.h"
#include "Sound/Resound/RepeatGrainData.h"
#include "Sound/Resound/RandomGrainData.h"
#include "Sound/Resound/SequenceGrainData.h"
#include "Sound/Resound/SimultaneousGrainData.h"
#include "Sound/Resound/TriggerGrainData.h"
#include "Sound/Editor/SoundCategory.h"
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
	if (const BlendGrainData* blendGrain = dynamic_type_cast< const BlendGrainData* >(grain))
	{
		const Ref< IGrainData >* grains = blendGrain->getGrains();
		T_ASSERT(grains);

		buildGrainDependencies(pipelineDepends, grains[0]);
		buildGrainDependencies(pipelineDepends, grains[1]);
	}

	if (const EnvelopeGrainData* envelopeGrain = dynamic_type_cast< const EnvelopeGrainData* >(grain))
	{
		const std::vector< EnvelopeGrainData::GrainData >& grains = envelopeGrain->getGrains();
		for (std::vector< EnvelopeGrainData::GrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
			buildGrainDependencies(pipelineDepends, i->grain);
	}

	if (const InLoopOutGrainData* iloGrain = dynamic_type_cast< const InLoopOutGrainData* >(grain))
	{
		buildGrainDependencies(pipelineDepends, iloGrain->getInGrain());
		buildGrainDependencies(pipelineDepends, iloGrain->getInLoopGrain());
		buildGrainDependencies(pipelineDepends, iloGrain->getOutGrain());
		buildGrainDependencies(pipelineDepends, iloGrain->getOutLoopGrain());
	}

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

	if (const SimultaneousGrainData* simultaneousGrain = dynamic_type_cast< const SimultaneousGrainData* >(grain))
	{
		const RefArray< IGrainData >& grains = simultaneousGrain->getGrains();
		for (RefArray< IGrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
			buildGrainDependencies(pipelineDepends, *i);
	}

	if (const TriggerGrainData* triggerGrain = dynamic_type_cast< const TriggerGrainData* >(grain))
		buildGrainDependencies(pipelineDepends, triggerGrain->getGrain());

	if (const PlayGrainData* playGrain = dynamic_type_cast< const PlayGrainData* >(grain))
		pipelineDepends->addDependency(playGrain->getSound(), editor::PdfBuild | editor::PdfResource);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.BankPipeline", 10, BankPipeline, editor::DefaultPipeline)

TypeInfoSet BankPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< BankAsset >();
	return typeSet;
}

bool BankPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const BankAsset* bankAsset = checked_type_cast< const BankAsset*, false >(sourceAsset);

	const RefArray< IGrainData >& grains = bankAsset->getGrains();
	for (RefArray< IGrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
		buildGrainDependencies(pipelineDepends, *i);

	Ref< const SoundCategory > category = pipelineDepends->getObjectReadOnly< SoundCategory >(bankAsset->m_category);
	if (category)
		pipelineDepends->addDependency(bankAsset->m_category, editor::PdfUse);

	while (category)
	{
		Ref< const SoundCategory > parent = pipelineDepends->getObjectReadOnly< SoundCategory >(category->getParent());
		if (parent)
			pipelineDepends->addDependency(category->getParent(), editor::PdfUse);

		category = parent;
	}

	return true;
}

bool BankPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const BankAsset* bankAsset = checked_type_cast< const BankAsset*, false >(sourceAsset);

	bool categorized = false;
	std::wstring configurationId;
	float gain = 0.0f;
	float range = 0.0f;

	Ref< const SoundCategory > category = pipelineBuilder->getObjectReadOnly< SoundCategory >(bankAsset->m_category);

	if (category)
		configurationId = category->getConfigurationId();

	while (category)
	{
		categorized = true;
		gain += category->getGain();
		range = max(range, category->getRange());
		category = pipelineBuilder->getObjectReadOnly< SoundCategory >(category->getParent());
	}

	if (categorized)
	{
		log::info << L"Category gain " << gain << L" dB" << Endl;
		log::info << L"         range " << range << Endl;
	}
	else
		log::warning << L"Uncategorized sound \"" << sourceInstance->getName() << L"\"" << Endl;

	Ref< BankResource > bankResource = new BankResource(
		bankAsset->m_grains,
		configurationId,
		gain,
		range
	);

	return editor::DefaultPipeline::buildOutput(
		pipelineBuilder,
		dependencySet,
		dependency,
		sourceInstance,
		bankResource,
		outputPath,
		outputGuid,
		buildParams,
		reason
	);
}

	}
}
