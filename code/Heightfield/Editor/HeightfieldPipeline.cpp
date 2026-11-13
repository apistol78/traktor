/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/HeightfieldResource.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldPipeline.h"
#include "Heightfield/Editor/HeightfieldTileAsset.h"
#include "Heightfield/Editor/TileStorage.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldPipeline", 5, HeightfieldPipeline, editor::IPipeline)

bool HeightfieldPipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

void HeightfieldPipeline::destroy()
{
	TileStorage::flushCache();
}

TypeInfoSet HeightfieldPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< HeightfieldAsset, HeightfieldTileAsset >();
}

bool HeightfieldPipeline::shouldCache() const
{
	return true;
}

uint32_t HeightfieldPipeline::hashAsset(const ISerializable* sourceAsset) const
{
	return DeepHash(sourceAsset).get();
}

bool HeightfieldPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	return true;
}

bool HeightfieldPipeline::buildOutput(
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
	Ref< const db::Instance > heightfieldInstance = sourceInstance;
	Ref< const HeightfieldAsset > heightfieldAsset;
	Ref< Heightfield > heightfield;

	if (auto tileAsset = dynamic_type_cast< const HeightfieldTileAsset* >(sourceAsset))
	{
		heightfieldInstance = pipelineBuilder->getSourceDatabase()->getInstance(tileAsset->m_heightfield);
		heightfieldAsset = heightfieldInstance ? heightfieldInstance->getObject< HeightfieldAsset >() : nullptr;
		if (!heightfieldAsset)
		{
			log::error << L"Heightfield pipeline failed; unable to get heightfield asset of tile." << Endl;
			return false;
		}

		const TileStorage tileStorage(heightfieldAsset->getTileCount());
		if (tileAsset->m_tileX >= 0)
			heightfield = tileStorage.readHeightfieldTile(heightfieldInstance, heightfieldAsset->getWorldExtent(), tileAsset->m_tileX, tileAsset->m_tileZ, 0);
		else
			heightfield = tileStorage.readHeightfieldOverview(heightfieldInstance, heightfieldAsset->getWorldExtent(), tileAsset->m_overviewSize);
	}
	else
	{
		heightfieldAsset = checked_type_cast< const HeightfieldAsset* >(sourceAsset);

		// Product of asset itself is a low resolution overview.
		heightfield = TileStorage(heightfieldAsset->getTileCount()).readHeightfieldOverview(sourceInstance, heightfieldAsset->getWorldExtent());
	}

	if (!heightfield)
	{
		log::error << L"Heightfield pipeline failed; unable to read heights." << Endl;
		return false;
	}

	// Create height field resource.
	Ref< HeightfieldResource > resource = new HeightfieldResource();

	// Create instance's name.
	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Heightfield pipeline failed; unable to create instance." << Endl;
		return false;
	}

	Ref< IStream > outputData = instance->writeData(L"Data");
	if (!outputData)
	{
		log::error << L"Heightfield pipeline failed; unable to create data stream." << Endl;
		instance->revert();
		return false;
	}

	HeightfieldFormat().write(outputData, heightfield);

	outputData->close();
	outputData = nullptr;

	resource->m_worldExtent = heightfield->getWorldExtent();
	resource->m_worldCenter = heightfield->getWorldCenter();

	instance->setObject(resource);

	return instance->commit();
}

Ref< ISerializable > HeightfieldPipeline::buildProduct(
	editor::IPipelineBuilder* pipelineBuilder,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const Object* buildParams
) const
{
	T_FATAL_ERROR;
	return nullptr;
}

}
