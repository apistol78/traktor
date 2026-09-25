/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Texture/TrimSheetTexturePipeline.h"

#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Render/Editor/Texture/TrimSheetComposer.h"
#include "Render/Editor/Texture/TrimSheetSetupAsset.h"
#include "Render/Editor/Texture/TrimSheetTextureAsset.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TrimSheetTexturePipeline", 0, TrimSheetTexturePipeline, editor::DefaultPipeline)

bool TrimSheetTexturePipeline::create(const editor::IPipelineSettings* settings, db::Database* database)
{
	m_assetPath = settings->getPropertyExcludeHash< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

TypeInfoSet TrimSheetTexturePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< TrimSheetTextureAsset >();
}

bool TrimSheetTexturePipeline::shouldCache() const
{
	return true;
}

bool TrimSheetTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const TrimSheetTextureAsset* asset = checked_type_cast< const TrimSheetTextureAsset*, false >(sourceAsset);

	Ref< const TrimSheetSetupAsset > setup = pipelineDepends->getObjectReadOnly< TrimSheetSetupAsset >(asset->getSetup());
	if (!setup)
	{
		log::error << L"Trim sheet texture pipeline failed; unable to read trim sheet setup " << asset->getSetup().format() << L"." << Endl;
		return false;
	}

	// Rebuild whenever setup is modified.
	pipelineDepends->addDependency(asset->getSetup(), editor::PdfUse);

	// Only source images of our layer affect the texture.
	std::set< std::wstring > files;
	TrimSheetComposer::collectFiles(setup, asset->getLayer(), files);
	for (const auto& file : files)
		pipelineDepends->addDependency(Path(m_assetPath), file);

	pipelineDepends->addDependency< TextureOutput >();
	return true;
}

bool TrimSheetTexturePipeline::buildOutput(
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
	const TrimSheetTextureAsset* asset = checked_type_cast< const TrimSheetTextureAsset*, false >(sourceAsset);

	Ref< const TrimSheetSetupAsset > setup = pipelineBuilder->getObjectReadOnly< TrimSheetSetupAsset >(asset->getSetup());
	if (!setup)
	{
		log::error << L"Trim sheet texture pipeline failed; unable to read trim sheet setup " << asset->getSetup().format() << L"." << Endl;
		return false;
	}

	TrimSheetComposer composer{ Path(m_assetPath) };
	Ref< drawing::Image > image = composer.compose(setup, asset->getLayer());
	if (!image)
	{
		log::error << L"Trim sheet texture pipeline failed; unable to compose trim sheet." << Endl;
		return false;
	}

	// Engine expect green of normal maps to point down in image.
	const bool inverseNormalMapY = (setup->getNormalConvention() == TrimSheetSetupAsset::NormalConvention::OpenGL);

	Ref< TextureOutput > output = createTextureOutput(asset, inverseNormalMapY);
	return pipelineBuilder->buildAdHocOutput(
		output,
		outputPath,
		outputGuid,
		image
	);
}

Ref< TextureOutput > TrimSheetTexturePipeline::createTextureOutput(const TrimSheetTextureAsset* asset, bool inverseNormalMapY)
{
	Ref< TextureOutput > output = new TextureOutput();
	output->m_textureType = Tt2D;
	output->m_generateMips = asset->getGenerateMips();
	output->m_enableCompression = asset->getEnableCompression();

	switch (asset->getLayer())
	{
	case TrimSheetLayer::Albedo:
		output->m_hasAlpha = asset->getKeepAlpha();
		output->m_ignoreAlpha = !asset->getKeepAlpha();
		break;

	case TrimSheetLayer::Specular:
		output->m_hasAlpha = asset->getKeepAlpha();
		output->m_ignoreAlpha = !asset->getKeepAlpha();
		output->m_assumeLinearGamma = true;
		break;

	case TrimSheetLayer::Roughness:
		// Materials sample roughness from red; exact single channel if not compressed.
		if (!asset->getEnableCompression())
			output->m_textureFormat = TfR8;
		output->m_ignoreAlpha = true;
		output->m_assumeLinearGamma = true;
		break;

	case TrimSheetLayer::Normal:
		output->m_normalMap = true;
		output->m_inverseNormalMapY = inverseNormalMapY;
		output->m_ignoreAlpha = true;
		output->m_assumeLinearGamma = true;
		break;

	case TrimSheetLayer::Height:
		output->m_textureFormat = TfR8;
		output->m_ignoreAlpha = true;
		output->m_assumeLinearGamma = true;
		break;
	}

	return output;
}

}
