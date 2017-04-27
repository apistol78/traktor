/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if defined(_WIN32)
#	include <CCubeMapProcessor.h>
#	include <ErrorMsg.h>
#endif
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Editor/Texture/CubeMap.h"
#include "Render/Editor/Texture/ProbeTextureAsset.h"
#include "Render/Editor/Texture/ProbeTexturePipeline.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProbeTexturePipeline", 1, ProbeTexturePipeline, editor::DefaultPipeline)

bool ProbeTexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

TypeInfoSet ProbeTexturePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ProbeTextureAsset >());
	return typeSet;
}

bool ProbeTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const ProbeTextureAsset* asset = checked_type_cast< const ProbeTextureAsset* >(sourceAsset);
	pipelineDepends->addDependency(Path(m_assetPath), asset->getFileName().getOriginal());
	pipelineDepends->addDependency< TextureOutput >();
	return true;
}

bool ProbeTexturePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
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
	const ProbeTextureAsset* asset = checked_type_cast< const ProbeTextureAsset* >(sourceAsset);
	Ref< TextureOutput > output;

	Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), asset->getFileName().getOriginal());
	if (!file)
	{
		log::error << L"Probe texture asset pipeline failed; unable to open source image \"" << asset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	Ref< drawing::Image > image = drawing::Image::load(file, asset->getFileName().getExtension());
	if (!image)
	{
		log::error << L"Probe texture asset pipeline failed; unable to load source image \"" << asset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	file->close();

	// Ensure source image has high dynamic range.
	image->convert(drawing::PixelFormat::getARGBF32());

#if defined(_WIN32)
	Ref< CubeMap > cm = new CubeMap(image);
	int32_t size = cm->getSize();

	CCubeMapProcessor cubeMapProcessor;
	cubeMapProcessor.Init(size, size, 1, 4);

	for (int32_t i = 0; i < 6; ++i)
	{
		cubeMapProcessor.SetInputFaceData(
			i,
			CP_VAL_FLOAT32,
			4,
			size * 4 * sizeof(float),
			(void*)cm->getSide(i)->getData(),
			1000000.0f,
			1.0f,
			1.0f
		);
	}

	cubeMapProcessor.InitiateFiltering(
		asset->m_filterAngle,
		asset->m_filterMipDeltaAngle,
		1.0f,
		CP_FILTER_TYPE_ANGULAR_GAUSSIAN,
		CP_FIXUP_PULL_LINEAR,
		3,
		TRUE
	);

	while (cubeMapProcessor.GetStatus() == CP_STATUS_PROCESSING)
	{
		log::debug << cubeMapProcessor.GetFilterProgressString() << Endl;
		ThreadManager::getInstance().getCurrentThread()->sleep(200);
	}

	for (int32_t i = 0; i < 6; ++i)
	{
		std::memcpy(
			(void*)cm->getSide(i)->getData(),
			cubeMapProcessor.m_OutputSurface[0][i].m_ImgData,
			size * size * 4 * sizeof(float)
		);
	}

	Ref< drawing::Image > cross = cm->createCrossImage();
	if (!cross)
	{
		log::error << L"Probe texture asset pipeline failed; unable to create cross image." << Endl;
		return false;
	}
#else
	// \fixme Since CubeMapGen doesn't compile on non-windows we cannot
	// filter probe yet on Linux nor OSX.
	Ref< drawing::Image > cross = image;
#endif

	output = new TextureOutput();
	output->m_textureFormat = TfInvalid;
	output->m_generateMips = true;
	output->m_keepZeroAlpha = false;
	output->m_textureType = TtCube;
	output->m_hasAlpha = false;
	output->m_enableCompression = true;
	output->m_linearGamma = true;

	return pipelineBuilder->buildOutput(
		output,
		outputPath,
		outputGuid,
		cross
	);
}

	}
}
