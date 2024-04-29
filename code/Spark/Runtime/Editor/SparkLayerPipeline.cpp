/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/IPipelineDepends.h"
#include "Spark/Runtime/SparkLayerData.h"
#include "Spark/Runtime/Editor/SparkLayerPipeline.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SparkLayerPipeline", 0, SparkLayerPipeline, editor::DefaultPipeline)

TypeInfoSet SparkLayerPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< SparkLayerData >();
}

bool SparkLayerPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const SparkLayerData* layerData = mandatory_non_null_type_cast< const SparkLayerData* >(sourceAsset);

	pipelineDepends->addDependency(layerData->m_movie, editor::PdfBuild | editor::PdfResource);

	for (const auto& externalMovie : layerData->m_externalMovies)
		pipelineDepends->addDependency(externalMovie.second, editor::PdfBuild | editor::PdfResource);

	return true;
}

}
