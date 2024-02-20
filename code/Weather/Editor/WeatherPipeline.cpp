/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/IPipelineDepends.h"
#include "Weather/Clouds/CloudComponentData.h"
#include "Weather/Editor/WeatherPipeline.h"
#include "Weather/Precipitation/PrecipitationComponentData.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor::weather
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.weather.WeatherPipeline", 0, WeatherPipeline, world::EntityPipeline)

TypeInfoSet WeatherPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< CloudComponentData >();
	typeSet.insert< PrecipitationComponentData >();
	typeSet.insert< SkyComponentData >();
	return typeSet;
}

bool WeatherPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const CloudComponentData* cloudComponentData = dynamic_type_cast< const CloudComponentData* >(sourceAsset))
	{
		pipelineDepends->addDependency(cloudComponentData->getParticleShader(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(cloudComponentData->getParticleTexture(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(cloudComponentData->getImpostorShader(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(cloudComponentData->getMask(), editor::PdfBuild | editor::PdfResource);
	}
	else if (const PrecipitationComponentData* precipitationComponentData = dynamic_type_cast< const PrecipitationComponentData* >(sourceAsset))
		pipelineDepends->addDependency(precipitationComponentData->getMesh(), editor::PdfBuild | editor::PdfResource);
	else if (const SkyComponentData* skyComponentData = dynamic_type_cast< const SkyComponentData* >(sourceAsset))
	{
		const Guid c_shaderClouds2D(L"{9F52BE0A-0C1A-4928-91D9-9D32296CB8F3}");
		const Guid c_shaderClouds3D(L"{EF88CE37-0917-4402-B2D1-6E3F2D3CCCCF}");
		const Guid c_shaderCloudsDome(L"{151F822B-B85F-6349-B536-7663C95C43B8}");

		pipelineDepends->addDependency(skyComponentData->getShader(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(skyComponentData->getTexture(), editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(c_shaderClouds2D, editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(c_shaderClouds3D, editor::PdfBuild | editor::PdfResource);
		pipelineDepends->addDependency(c_shaderCloudsDome, editor::PdfBuild | editor::PdfResource);
	}
	return true;
}

}
