#include "Core/Log/Log.h"
#include "Core/Settings/PropertyString.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldFormat.h"
#include "Heightfield/Editor/HeightfieldTextureAsset.h"
#include "Heightfield/Editor/HeightfieldTexturePipeline.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor
{
	namespace hf
	{
		namespace
		{

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
		h[i] = heightfield->getGridHeightNearest(u + directions[i][0], v + directions[i][1]);

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldTexturePipeline", 3, HeightfieldTexturePipeline, editor::DefaultPipeline)

bool HeightfieldTexturePipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
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
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	const HeightfieldTextureAsset* asset = checked_type_cast< const HeightfieldTextureAsset* >(sourceAsset);

	// Get heightfield asset.
	Ref< const HeightfieldAsset > heightfieldAsset = pipelineBuilder->getObjectReadOnly< HeightfieldAsset >(asset->m_heightfield);
	if (!heightfieldAsset)
	{
		log::error << L"Heightfield texture pipeline failed; unable to read heightfield asset" << Endl;
		return false;
	}

	// Load heightfield from source file.
	Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), heightfieldAsset->getFileName().getOriginal());
	if (!file)
	{
		log::error << L"Heightfield texture pipeline failed; unable to open source (" << heightfieldAsset->getFileName().getOriginal() << L")" << Endl;
		return false;
	}

	Ref< Heightfield > heightfield = HeightfieldFormat().read(
		file,
		heightfieldAsset->getFileName().getExtension(),
		heightfieldAsset->getWorldExtent(),
		heightfieldAsset->getInvertX(),
		heightfieldAsset->getInvertZ(),
		heightfieldAsset->getDetailSkip()
	);
	if (!heightfield)
	{
		log::error << L"Heightfield texture pipeline failed; unable to read heightfield source \"" << heightfieldAsset->getFileName().getOriginal() << L"\"" << Endl;
		return 0;
	}

	uint32_t size = heightfield->getSize();

	if (asset->m_output == HeightfieldTextureAsset::OtHeights)
	{
		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getR16F(), size, size);

		for (int32_t v = 0; v < size; ++v)
		{
			for (int32_t u = 0; u < size; ++u)
			{
				float height = heightfield->getGridHeightNearest(u, v) * asset->m_scale;
				outputMap->setPixelUnsafe(u, v, Color4f(height, height, height, height));
			}
		}

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = render::TfR16F;
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

		return pipelineBuilder->buildOutput(
			output,
			outputPath,
			outputGuid,
			outputMap
		);
	}
	else	// OtNormals
	{
		Ref< drawing::Image > outputMap = new drawing::Image(drawing::PixelFormat::getX8R8G8B8(), size, size);
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
		output->m_enableCompression = true;
		output->m_enableNormalMapCompression = true;
		output->m_inverseNormalMapY = false;
		output->m_linearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;

		return pipelineBuilder->buildOutput(
			output,
			outputPath,
			outputGuid,
			outputMap
		);
	}
}

	}
}
