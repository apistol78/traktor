/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Acquire.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldTextureAsset.h"
#include "Heightfield/Editor/HeightfieldTexturePipeline.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

float fract(float v)
{
	return v - std::floor(v);
}

Vector4 normalAt(const Heightfield* heightfield, int32_t u, int32_t v)
{
	const float c_distance = 0.5f;
	const float directions[][2] =
	{
		{ -c_distance, -c_distance },
		{        0.0f, -c_distance },
		{  c_distance, -c_distance },
		{  c_distance,        0.0f },
		{  c_distance,  c_distance },
		{        0.0f,        0.0f },
		{ -c_distance,  c_distance },
		{ -c_distance,        0.0f }
	};

	float h0 = heightfield->getGridHeightNearest(u, v);

	float h[sizeof_array(directions)];
	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
		h[i] = heightfield->getGridHeightBilinear(u + directions[i][0], v + directions[i][1]);

	const Vector4& worldExtent = heightfield->getWorldExtent();
	float sx = worldExtent.x() / heightfield->getSize();
	float sy = worldExtent.y();
	float sz = worldExtent.z() / heightfield->getSize();

	Vector4 N = Vector4::zero();

	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
	{
		uint32_t j = (i + 1) % sizeof_array(directions);

		float dx1 = directions[i][0] * sx;
		float dy1 = (h[i] - h0) * sy;
		float dz1 = directions[i][1] * sz;

		float dx2 = directions[j][0] * sx;
		float dy2 = (h[j] - h0) * sy;
		float dz2 = directions[j][1] * sz;

		Vector4 n = cross(
			Vector4(dx2, dy2, dz2),
			Vector4(dx1, dy1, dz1)
		);

		N += n;
	}

	return N.normalized();
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldTexturePipeline", 5, HeightfieldTexturePipeline, editor::DefaultPipeline)

HeightfieldTexturePipeline::HeightfieldTexturePipeline()
:	m_heightFormat(HfFloat16)
,	m_compressNormals(false)
{
}

bool HeightfieldTexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	
	std::wstring hf = settings->getProperty< std::wstring >(L"HeightfieldTexturePipeline.HeightFormat", L"HtFloat16");
	if (hf == L"HtFloat16")
		m_heightFormat = HfFloat16;
	else if (hf == L"HfFloat32")
		m_heightFormat = HfFloat32;
	else if (hf == L"HfARGBEncoded")
		m_heightFormat = HfARGBEncoded;
	else
	{
		log::error << L"Heightfield pipeline error; Unknown height format \"" << hf << L"\"." << Endl;
		return false;
	}

	m_compressNormals = settings->getProperty< bool >(L"HeightfieldTexturePipeline.CompressNormals", false);
	return true;
}

TypeInfoSet HeightfieldTexturePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< HeightfieldTextureAsset >());
	return typeSet;
}

bool HeightfieldTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const HeightfieldTextureAsset* asset = checked_type_cast< const HeightfieldTextureAsset* >(sourceAsset);
	pipelineDepends->addDependency(asset->m_heightfield, editor::PdfUse);
	pipelineDepends->addDependency< render::TextureOutput >();
	return true;
}

bool HeightfieldTexturePipeline::buildOutput(
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
	// Due to memory usage of this pipeline we need to serialize
	// building of these assets. Pipeline should try reduce number
	// of concurrent builds with same pipeline type.
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const HeightfieldTextureAsset* asset = checked_type_cast< const HeightfieldTextureAsset* >(sourceAsset);

	// Get heightfield asset and instance.
	Ref< const db::Instance > heightfieldAssetInstance = pipelineBuilder->getSourceDatabase()->getInstance(asset->m_heightfield);
	if (!heightfieldAssetInstance)
	{
		log::error << L"Heightfield texture pipeline failed; unable to get heightfield asset instance" << Endl;
		return false;
	}

	Ref< const HeightfieldAsset > heightfieldAsset = heightfieldAssetInstance->getObject< const HeightfieldAsset >();
	if (!heightfieldAsset)
	{
		log::error << L"Heightfield texture pipeline failed; unable to get heightfield asset" << Endl;
		return false;
	}

	Ref< IStream > sourceData = heightfieldAssetInstance->readData(L"Data");
	if (!sourceData)
	{
		log::error << L"Heightfield pipeline failed; unable to open heights" << Endl;
		return false;
	}

	Ref< Heightfield > heightfield = HeightfieldFormat().read(
		sourceData,
		heightfieldAsset->getWorldExtent()
	);
	if (!heightfield)
	{
		log::error << L"Heightfield pipeline failed; unable to read heights" << Endl;
		return 0;
	}

	sourceData->close();
	sourceData = 0;

	int32_t size = heightfield->getSize();

	if (asset->m_output == HeightfieldTextureAsset::OtHeights)
	{
		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getR16F(), size, size);

		for (int32_t v = 0; v < size; ++v)
		{
			for (int32_t u = 0; u < size; ++u)
			{
				float height = heightfield->getGridHeightNearest(u, v) * asset->m_scale;
				if (m_heightFormat == HfARGBEncoded)
				{
					Vector4 enc = Vector4(
						fract(1.0f * height),
						fract(255.0f * height),
						fract(65025.0f * height),
						fract(160581375.0f * height)
					);
					enc -= enc.shuffle< 1, 2, 3, 3 >() * Vector4(1.0 / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 0.0f);
					outputMap->setPixelUnsafe(u, v, Color4f(enc));
				}
				else
					outputMap->setPixelUnsafe(u, v, Color4f(height, height, height, height));
			}
		}

		const render::TextureFormat c_heightTextureFormats[] = { render::TfR16F, render::TfR32F, render::TfR8G8B8A8 };

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = c_heightTextureFormats[m_heightFormat];
		output->m_generateNormalMap = false;
		output->m_scaleDepth = 0.0f;
		output->m_generateMips = false;
		output->m_keepZeroAlpha = false;
		output->m_textureType = render::Tt2D;
		output->m_hasAlpha = bool(m_heightFormat == HfARGBEncoded);
		output->m_ignoreAlpha = false;
		output->m_scaleImage = false;
		output->m_scaleWidth = 0;
		output->m_scaleHeight = 0;
		output->m_enableCompression = false;
		output->m_enableNormalMapCompression = false;
		output->m_inverseNormalMapY = false;
		output->m_linearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;
		output->m_sharpenRadius = 0;

		return pipelineBuilder->buildOutput(
			output,
			outputPath,
			outputGuid,
			outputMap
		);
	}
	else if (asset->m_output == HeightfieldTextureAsset::OtNormals)
	{
		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getR8G8B8(), size, size);
		for (int32_t v = 0; v < size; ++v)
		{
			for (int32_t u = 0; u < size; ++u)
			{
				Vector4 normal = normalAt(heightfield, u, v);
				normal = normal * Vector4(0.5f, 0.5f, 0.5f, 0.0f) + Vector4(0.5f, 0.5f, 0.5f, 0.0f);
				outputMap->setPixelUnsafe(u, v, Color4f(
					normal.x(),
					normal.y(),
					normal.z()
				));
			}
		}

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = render::TfInvalid;
		output->m_generateNormalMap = false;
		output->m_scaleDepth = 0.0f;
		output->m_generateMips = true;
		output->m_keepZeroAlpha = false;
		output->m_textureType = render::Tt2D;
		output->m_hasAlpha = false;
		output->m_ignoreAlpha = true;
		output->m_scaleImage = false;
		output->m_scaleWidth = 0;
		output->m_scaleHeight = 0;
		output->m_enableCompression = m_compressNormals;
		output->m_enableNormalMapCompression = m_compressNormals;
		output->m_inverseNormalMapY = false;
		output->m_linearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;
		output->m_sharpenRadius = 0;

		return pipelineBuilder->buildOutput(
			output,
			outputPath,
			outputGuid,
			outputMap
		);
	}
	else if (asset->m_output == HeightfieldTextureAsset::OtCuts)
	{
		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getR8(), size, size);

		for (int32_t v = 0; v < size; ++v)
		{
			for (int32_t u = 0; u < size; ++u)
			{
				float cut = heightfield->getGridCut(u, v) ? 1.0f : 0.0f;
				outputMap->setPixelUnsafe(u, v, Color4f(cut, cut, cut, cut));
			}
		}

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = render::TfR8;
		output->m_generateNormalMap = false;
		output->m_scaleDepth = 0.0f;
		output->m_generateMips = false;
		output->m_keepZeroAlpha = false;
		output->m_textureType = render::Tt2D;
		output->m_hasAlpha = false;
		output->m_ignoreAlpha = false;
		output->m_scaleImage = false;
		output->m_scaleWidth = 0;
		output->m_scaleHeight = 0;
		output->m_enableCompression = false;
		output->m_enableNormalMapCompression = false;
		output->m_inverseNormalMapY = false;
		output->m_linearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;
		output->m_sharpenRadius = 0;

		return pipelineBuilder->buildOutput(
			output,
			outputPath,
			outputGuid,
			outputMap
		);
	}
	else if (asset->m_output == HeightfieldTextureAsset::OtUnwrap)
	{
		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getRGBAF32(), size, size);

		const Vector4& worldExtent = heightfield->getWorldExtent();
		for (int32_t v = 0; v < size; ++v)
		{
			for (int32_t u = 0; u < size; ++u)
			{
				float h = heightfield->getGridHeightNearest(u, v) * worldExtent.y();
				float hu = heightfield->getGridHeightNearest(u + 1, v) * worldExtent.y();
				float hv = heightfield->getGridHeightNearest(u, v + 1) * worldExtent.y();
				outputMap->setPixelUnsafe(u, v, Color4f(abs(hu - h), abs(hv - h), 0.0f, 1.0f));
			}
		}

		Color4f ct;

		for (int32_t v = 0; v < size; ++v)
		{
			Scalar total(0.0f);
			for (int32_t u = 0; u < size; ++u)
			{
				outputMap->getPixelUnsafe(u, v, ct);

				Scalar du = ct.getRed();

				ct.setRed(total);
				outputMap->setPixelUnsafe(u, v, ct);

				total += du * Scalar(0.5f) + Scalar(0.5f);
			}
			for (int32_t u = 0; u < size; ++u)
			{
				outputMap->getPixelUnsafe(u, v, ct);
				ct.setRed(ct.getRed() / total);
				outputMap->setPixelUnsafe(u, v, ct);
			}
		}

		for (int32_t u = 0; u < size; ++u)
		{
			Scalar total(0.0f);
			for (int32_t v = 0; v < size; ++v)
			{
				outputMap->getPixelUnsafe(u, v, ct);

				Scalar dv = ct.getGreen();

				ct.setGreen(total);
				outputMap->setPixelUnsafe(u, v, ct);

				total += dv * Scalar(0.5f) + Scalar(0.5f);
			}
			for (int32_t v = 0; v < size; ++v)
			{
				outputMap->getPixelUnsafe(u, v, ct);
				ct.setGreen(ct.getGreen() / total);
				outputMap->setPixelUnsafe(u, v, ct);
			}
		}

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = render::TfR8G8B8A8;
		output->m_generateNormalMap = false;
		output->m_scaleDepth = 0.0f;
		output->m_generateMips = false;
		output->m_keepZeroAlpha = false;
		output->m_textureType = render::Tt2D;
		output->m_hasAlpha = false;
		output->m_ignoreAlpha = false;
		output->m_scaleImage = false;
		output->m_scaleWidth = 0;
		output->m_scaleHeight = 0;
		output->m_enableCompression = false;
		output->m_enableNormalMapCompression = false;
		output->m_inverseNormalMapY = false;
		output->m_linearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;
		output->m_sharpenRadius = 0;

		return pipelineBuilder->buildOutput(
			output,
			outputPath,
			outputGuid,
			outputMap
		);
	}
	else
		return false;
}

	}
}
