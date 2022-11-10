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
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Sound/Tracker/PatternData.h"
#include "Sound/Tracker/PlayData.h"
#include "Sound/Tracker/SongResource.h"
#include "Sound/Tracker/TrackData.h"
#include "Sound/Editor/SoundCategory.h"
#include "Sound/Editor/Tracker/SongAsset.h"
#include "Sound/Editor/Tracker/SongPipeline.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SongPipeline", 0, SongPipeline, editor::DefaultPipeline)

TypeInfoSet SongPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< SongAsset >();
}

bool SongPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const SongAsset* songAsset = checked_type_cast< const SongAsset*, false >(sourceAsset);

	// Add dependencies to instruments.
	for (const auto& pattern : songAsset->m_patterns)
	{
		for (const auto& track : pattern->getTracks())
		{
			for (const auto& key : track->getKeys())
			{
				if (key.play)
					pipelineDepends->addDependency(key.play->getSound(), editor::PdfBuild | editor::PdfResource);
			}
		}
	}

	// Add dependencies to categories.
	Ref< const SoundCategory > category = pipelineDepends->getObjectReadOnly< SoundCategory >(songAsset->m_category);
	if (category)
		pipelineDepends->addDependency(songAsset->m_category, editor::PdfUse);

	while (category)
	{
		Ref< const SoundCategory > parent = pipelineDepends->getObjectReadOnly< SoundCategory >(category->getParent());
		if (parent)
			pipelineDepends->addDependency(category->getParent(), editor::PdfUse);

		category = parent;
	}

	return true;
}

bool SongPipeline::buildOutput(
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
	const SongAsset* songAsset = mandatory_non_null_type_cast< const SongAsset* >(sourceAsset);

	bool categorized = false;
	std::wstring configurationId;
	float gain = 0.0f;
	float range = 0.0f;

	Ref< const SoundCategory > category = pipelineBuilder->getObjectReadOnly< SoundCategory >(songAsset->m_category);

	if (category)
		configurationId = category->getConfigurationId();

	while (category)
	{
		categorized = true;
		gain += category->getGain();
		range = std::max(range, category->getRange());
		category = pipelineBuilder->getObjectReadOnly< SoundCategory >(category->getParent());
	}

	if (categorized)
	{
		log::info << L"Category gain " << gain << L" dB" << Endl;
		log::info << L"         range " << range << Endl;
	}
	else
		log::warning << L"Uncategorized sound \"" << sourceInstance->getName() << L"\"" << Endl;

	Ref< SongResource > songResource = new SongResource(
		songAsset->m_patterns,
		configurationId,
		gain,
		range,
		songAsset->m_bpm
	);

	return editor::DefaultPipeline::buildOutput(
		pipelineBuilder,
		dependencySet,
		dependency,
		sourceInstance,
		songResource,
		outputPath,
		outputGuid,
		buildParams,
		reason
	);
}

	}
}
