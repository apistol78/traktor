#include <cstring>
#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Range.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Filters/TransformFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Editor/Texture/CubeMap.h"
#include "Render/Editor/Texture/DxtnCompressor.h"
#include "Render/Editor/Texture/EtcCompressor.h"
#include "Render/Editor/Texture/IrradianceProbeAsset.h"
#include "Render/Editor/Texture/ProbePipeline.h"
#include "Render/Editor/Texture/ProbeProcessor.h"
#include "Render/Editor/Texture/PvrtcCompressor.h"
#include "Render/Editor/Texture/RadianceProbeAsset.h"
#include "Render/Editor/Texture/UnCompressor.h"
#include "Render/Resource/TextureResource.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

Range< Scalar > measureDynamicRange(const CubeMap* cubeMap)
{
	Scalar mn( std::numeric_limits< float >::max());
	Scalar mx(-std::numeric_limits< float >::max());
	for (int32_t side = 0; side < 6; ++side)
	{
		const drawing::Image* mip = cubeMap->getSide(side);
		for (int32_t y = 0; y < mip->getHeight(); ++y)
		{
			for (int32_t x = 0; x < mip->getWidth(); ++x)
			{
				Color4f c;
				mip->getPixelUnsafe(x, y, c);
				Scalar a = Vector4(c).shuffle< 0, 1, 2, 0 >().max();
				mn = min(mn, a);
				mx = max(mx, a);
			}
		}
	}
	T_ASSERT(mn <= mx);
	return Range< Scalar >(mn, mx);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProbePipeline", 1, ProbePipeline, editor::DefaultPipeline)

ProbePipeline::ProbePipeline()
:	m_processor(new ProbeProcessor())
{
}

bool ProbePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	if (!m_processor->create())
		return false;

	return true;
}

void ProbePipeline::destroy()
{
	safeDestroy(m_processor);
	editor::DefaultPipeline::destroy();
}

TypeInfoSet ProbePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< IrradianceProbeAsset >());
	typeSet.insert(&type_of< RadianceProbeAsset >());
	return typeSet;
}

bool ProbePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	 if (const IrradianceProbeAsset* irradianceAsset = dynamic_type_cast< const IrradianceProbeAsset* >(sourceAsset))
		pipelineDepends->addDependency(Path(m_assetPath), irradianceAsset->getFileName().getOriginal());
	else if (const RadianceProbeAsset* radianceAsset = dynamic_type_cast< const RadianceProbeAsset* >(sourceAsset))
		pipelineDepends->addDependency(Path(m_assetPath), radianceAsset->getFileName().getOriginal());
	else
		return false;

	return true;
}

bool ProbePipeline::buildOutput(
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
	const editor::Asset* asset = checked_type_cast< const editor::Asset* >(sourceAsset);

	Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), asset->getFileName().getOriginal());
	if (!file)
	{
		log::error << L"Probe texture asset pipeline failed; unable to open source image \"" << asset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	Ref< drawing::Image > assetImage = drawing::Image::load(file, asset->getFileName().getExtension());
	if (!assetImage)
	{
		log::error << L"Probe texture asset pipeline failed; unable to load source image \"" << asset->getFileName().getOriginal() << L"\"" << Endl;
		return false;
	}

	file->close();

	RefArray< CubeMap > cubeMips;

	if (const IrradianceProbeAsset* irradianceAsset = dynamic_type_cast< const IrradianceProbeAsset* >(asset))
	{
		if (!m_processor->irradiance(assetImage, irradianceAsset->getFactor(), 256, cubeMips))
			return false;
	}
	else if (const RadianceProbeAsset* radianceAsset = dynamic_type_cast< const RadianceProbeAsset* >(asset))
	{
		// Scale asset image if required.
		if (radianceAsset->getSizeDivisor() > 1)
		{
			drawing::ScaleFilter scaleFilter(
				assetImage->getWidth() / radianceAsset->getSizeDivisor(),
				assetImage->getHeight() / radianceAsset->getSizeDivisor(),
				drawing::ScaleFilter::MnAverage,
				drawing::ScaleFilter::MgLinear
			);
			assetImage->apply(&scaleFilter);
		}

		if (!m_processor->radiance(assetImage, radianceAsset->getGlossScale(), radianceAsset->getGlossBias(), cubeMips))
			return false;
	}

	if (cubeMips.empty())
		return false;

	// Divide (ir-)radiance by PI.
	const Scalar nf(1.0f / PI);
	const drawing::TransformFilter tf(
		Color4f(nf, nf, nf, 1.0f),
		Color4f(0.0f, 0.0f, 0.0f, 0.0f)
	);
	for (auto cubeMip : cubeMips)
	{
		for (int32_t side = 0; side < 6; ++side)
			cubeMip->getSide(side)->apply(&tf);
	}		

	const uint32_t sideSize = cubeMips.front()->getSize();
	const uint32_t mipCount = uint32_t(cubeMips.size());

	// Measure probe mips.
	log::info << L"Probe dynamic range (per mip)" << Endl;
	log::info << IncreaseIndent;
	for (uint32_t i = 0; i < mipCount; ++i)
	{
		auto r = measureDynamicRange(cubeMips[i]);
		log::info << i << L". " << r.delta() << L" (min " << r.min << L", max " << r.max << L")" << Endl;
	}
	log::info << DecreaseIndent;

	// Create output instance.
	Ref< TextureResource > outputResource = new TextureResource();
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance." << Endl;
		return false;
	}

	outputInstance->setObject(outputResource);

	// Create output data stream.
	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Unable to create texture data stream." << Endl;
		outputInstance->revert();
		return false;
	}

	Writer writer(stream);
	writer << uint32_t(12);
	writer << int32_t(sideSize);
	writer << int32_t(sideSize);
	writer << int32_t(6);
	writer << int32_t(mipCount);
	writer << int32_t(TfR32G32B32A32F);
	writer << bool(false);
	writer << uint8_t(TtCube);
	writer << bool(true);
	writer << bool(true);

	// Create data writer, use deflate compression if enabled.
	Ref< IStream > streamData = new compress::DeflateStreamLzf(stream);
	Writer writerData(streamData);

	RefArray< drawing::Image > mipImages(mipCount);
	for (uint32_t side = 0; side < 6; ++side)
	{
		for (uint32_t i = 0; i < mipCount; ++i)
			mipImages[i] = cubeMips[i]->getSide(side);
		UnCompressor().compress(writerData, mipImages, TfR32G32B32A32F, false, 1);
	}

	streamData->close();

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

	}
}
