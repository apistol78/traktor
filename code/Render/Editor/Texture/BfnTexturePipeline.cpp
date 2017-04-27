/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Log/Log.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Render/Editor/Texture/BfnTextureAsset.h"
#include "Render/Editor/Texture/BfnTexturePipeline.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

float quantize255(float value)
{
	float q = std::floor(value * 127.5f + 127.5f);
	return 2.0f * q / 255.0f - 1.0f;
}

Vector4 findMinimumQuantizationError(const Vector4& unit)
{
	float minError = std::numeric_limits< float >::max();
	Vector4 out;

	for (int32_t i = 1; i < 128; ++i)
	{
		float t = float(i) / (128 - 1);

		Vector4 probe = unit * Scalar(t);
		Vector4 quantized(
			quantize255(probe.x()),
			quantize255(probe.y()),
			quantize255(probe.z())
		);
		Vector4 diff = (quantized - probe).absolute() / Scalar(t);

		float error = max< float >(diff.x(), diff.y(), diff.z());
		if (error < minError)
		{
			out = quantized;
			minError = error;
		}
	}

	return out;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.BfnTexturePipeline", 4, BfnTexturePipeline, editor::DefaultPipeline)

TypeInfoSet BfnTexturePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< BfnTextureAsset >());
	return typeSet;
}

bool BfnTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	pipelineDepends->addDependency< TextureOutput >();
	return true;
}

bool BfnTexturePipeline::buildOutput(
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
	const BfnTextureAsset* asset = checked_type_cast< const BfnTextureAsset*, false >(sourceAsset);

	Ref< drawing::Image > image;
	Ref< TextureOutput > output;

	if (asset->m_bestFitFactorOnly)
	{
		int32_t size = asset->m_size;

		if (!asset->m_collapseSymmetry)
		{
			image = new drawing::Image(drawing::PixelFormat::getR8(), 6 * size, size);
			image->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

			for (int32_t v = 0; v < size; ++v)
			{
				float nv = float(v) / (size - 1) * 2.0f - 1.0f;
				for (int32_t u = 0; u < size; ++u)
				{
					float nu = float(u) / (size - 1) * 2.0f - 1.0f;
					Vector4 n = Vector4(nu, 1.0f, nv).normalized();
					Vector4 qn = findMinimumQuantizationError(n);
					float qnln = qn.length();
				
					for (int32_t side = 0; side < 6; ++side)
						image->setPixel(u + side * size, v, Color4f(qnln, qnln, qnln, qnln));
				}
			}

			output = new TextureOutput();
			output->m_textureFormat = TfR8;
			output->m_generateMips = false;
			output->m_keepZeroAlpha = false;
			output->m_textureType = TtCube;
			output->m_enableCompression = false;
			output->m_linearGamma = true;
		}
		else
		{
			image = new drawing::Image(drawing::PixelFormat::getR8(), size, size);
			image->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

			for (int32_t v = 0; v < size; ++v)
			{
				float nv = float(v) / (size - 1) * 2.0f - 1.0f;
				for (int32_t u = 0; u < size; ++u)
				{
					float nu = float(u) / (size - 1) * 2.0f - 1.0f;
					Vector4 n = Vector4(nu, 1.0f, nv).normalized();
					Vector4 qn = findMinimumQuantizationError(n);
					float qnln = qn.length();
				
					image->setPixel(u, v, Color4f(qnln, qnln, qnln, qnln));
				}
			}

			output = new TextureOutput();
			output->m_textureFormat = TfR8;
			output->m_generateMips = false;
			output->m_keepZeroAlpha = false;
			output->m_textureType = Tt2D;
			output->m_enableCompression = false;
			output->m_linearGamma = true;
			output->m_sharpenRadius = 0;
		}
	}

	if (!image || !output)
	{
		log::error << L"Unable to generate BFN texture; invalid option(s)" << Endl;
		return 0;
	}

	return pipelineBuilder->buildOutput(
		output,
		outputPath,
		outputGuid,
		image
	);
}

	}
}
