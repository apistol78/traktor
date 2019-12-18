#include <functional>
#include "Core/Io/IStream.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/Random.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Editor/Texture/CubeMap.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"
#include "World/Editor/IrradianceGridAsset.h"
#include "World/Editor/IrradianceGridPipeline.h"
#include "World/IrradianceGridResource.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

class WrappedSHFunction : public render::SHFunction
{
public:
	WrappedSHFunction(const std::function< Vector4 (const Vector4&) >& fn)
	:	m_fn(fn)
	{
	}

	virtual Vector4 evaluate(float phi, float theta, const Vector4& unit) const override final
	{
		return m_fn(unit);
	}

private:
	std::function< Vector4 (const Vector4&) > m_fn;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.IrradianceGridPipeline", 0, IrradianceGridPipeline, editor::IPipeline)

bool IrradianceGridPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	return true;
}

void IrradianceGridPipeline::destroy()
{
}

TypeInfoSet IrradianceGridPipeline::getAssetTypes() const
{
	return makeTypeInfoSet< IrradianceGridAsset >();
}

bool IrradianceGridPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	return true;
}

bool IrradianceGridPipeline::buildOutput(
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
	const IrradianceGridAsset* asset = mandatory_non_null_type_cast< const IrradianceGridAsset* >(sourceAsset);

	Ref< IStream > file = pipelineBuilder->openFile(Path(m_assetPath), asset->getFileName().getOriginal());
	if (!file)
	{
		log::error << L"Irradiance grid pipeline failed; unable to open source file." << Endl;
		return false;
	}

	Ref< drawing::Image > skyImage = drawing::Image::load(file, asset->getFileName().getExtension());
	if (!skyImage)
	{
		log::error << L"Irradiance grid pipeline failed; unable to read source image." << Endl;
		return false;
	}

	safeClose(file);

	Ref< render::CubeMap > cubeMap = render::CubeMap::createFromImage(skyImage);
	if (!cubeMap)
	{
		log::error << L"Irradiance grid pipeline failed; unable to create cube map from image." << Endl;
		return false;
	}

	Random random;
	WrappedSHFunction shFunction([&] (const Vector4& unit) -> Vector4 {
		Color4f cl(0.0f, 0.0f, 0.0f, 0.0f);
		for (int32_t i = 0; i < 100; ++i)
		{
			Vector2 uv = Quasirandom::hammersley(i, 100, random);
			Vector4 direction = Quasirandom::uniformHemiSphere(uv, unit);
			cl += cubeMap->get(direction);
		}
		return cl / Scalar(100.0f);
	});

	Ref< render::SHCoeffs > shCoeffs = new render::SHCoeffs();
	
	render::SHEngine shEngine(3);
	shEngine.generateSamplePoints(10000);
	shEngine.generateCoefficients(&shFunction, *shCoeffs);

	Ref< world::IrradianceGridResource > outputResource = new world::IrradianceGridResource();
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"Irradiance grid pipeline failed; unable to create output instance." << Endl;
		return false;
	}

	outputInstance->setObject(outputResource);

	// Create output data stream.
	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Irradiance grid pipeline failed; unable to create texture data stream." << Endl;
		outputInstance->revert();
		return false;
	}

	Writer writer(stream);

	writer << uint32_t(2);

	writer << (uint32_t)1;	// width
	writer << (uint32_t)1;	// height
	writer << (uint32_t)1;	// depth

	writer << (float)-10000.0f;
	writer << (float)-10000.0f;
	writer << (float)-10000.0f;
	writer << (float)10000.0f;
	writer << (float)10000.0f;
	writer << (float)10000.0f;

	for (int32_t i = 0; i < 9; ++i)
	{
		auto c = (*shCoeffs)[i];
		writer << c.x();
		writer << c.y();
		writer << c.z();
	}

	stream->close();

	if (!outputInstance->commit())
	{
		log::error << L"Irradiance grid pipeline failed; unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > IrradianceGridPipeline::buildOutput(
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
}
