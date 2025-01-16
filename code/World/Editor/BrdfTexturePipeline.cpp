/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Editor/BrdfTexturePipeline.h"

#include "Core/Log/Log.h"
#include "Core/Math/Quasirandom.h"
#include "Drawing/Image.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "World/Editor/BrdfTextureAsset.h"

#include <limits>

namespace traktor::world
{
namespace
{

Vector4 importanceSampleGGX(const Vector2& Xi, const Vector4& N, float roughness)
{
	const float a = roughness * roughness;

	const float phi = 2.0f * PI * Xi.x;
	const float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
	const float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

	// from spherical coordinates to Cartesian coordinates.
	const Vector4 H(
		cos(phi) * sinTheta,
		sin(phi) * sinTheta,
		cosTheta);

	// from tangent-space vector to world-space sample vector
	const Vector4 up = abs(N.z()) < 0.999f ? Vector4(0.0f, 0.0f, 1.0f) : Vector4(1.0f, 0.0f, 0.0f);
	const Vector4 tangent = cross(up, N).normalized();
	const Vector4 bitangent = cross(N, tangent);

	const Vector4 sampleVec = tangent * H.x() + bitangent * H.y() + N * H.z();
	return sampleVec.normalized();
}

float geometrySchlickGGX(float NdotV, float roughness)
{
	const float a = roughness;
	const float k = (a * a) / 2.0f;
	const float nom = NdotV;
	const float denom = NdotV * (1.0f - k) + k;
	return nom / denom;
}

float geometrySmith(const Vector4& N, const Vector4& V, const Vector4& L, float roughness)
{
	const float NdotV = std::max< float >(dot3(N, V), 0.0f);
	const float NdotL = std::max< float >(dot3(N, L), 0.0f);
	const float ggx2 = geometrySchlickGGX(NdotV, roughness);
	const float ggx1 = geometrySchlickGGX(NdotL, roughness);
	return ggx1 * ggx2;
}

Vector4 integrateBRDF(float NdotV, float roughness)
{
	Vector4 V(
		sqrt(1.0f - NdotV * NdotV),
		0.0f,
		NdotV);

	float A = 0.0f;
	float B = 0.0f;

	const Vector4 N = Vector4(0.0f, 0.0f, 1.0f);

	const uint32_t SAMPLE_COUNT = 1024u;
	for (uint32_t i = 0u; i < SAMPLE_COUNT; ++i)
	{
		const Vector2 Xi = Quasirandom::hammersley(i, SAMPLE_COUNT);
		const Vector4 H = importanceSampleGGX(Xi, N, roughness);
		const Vector4 L = (2.0_simd * dot3(V, H) * H - V).normalized();

		const float NdotL = std::max< float >(L.z(), 0.0f);
		const float NdotH = std::max< float >(H.z(), 0.0f);
		const float VdotH = std::max< float >(dot3(V, H), 0.0f);

		if (NdotL > 0.0f)
		{
			const float G = geometrySmith(N, V, L, roughness);
			const float G_Vis = (G * VdotH) / (NdotH * NdotV);
			const float Fc = pow(1.0f - VdotH, 5.0f);

			A += (1.0f - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	A /= float(SAMPLE_COUNT);
	B /= float(SAMPLE_COUNT);
	return Vector4(A, B, 0.0f);
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.BrdfTexturePipeline", 2, BrdfTexturePipeline, editor::DefaultPipeline)

TypeInfoSet BrdfTexturePipeline::getAssetTypes() const
{
	return makeTypeInfoSet< BrdfTextureAsset >();
}

bool BrdfTexturePipeline::shouldCache() const
{
	return true;
}

bool BrdfTexturePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid) const
{
	pipelineDepends->addDependency< render::TextureOutput >();
	return true;
}

bool BrdfTexturePipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::PipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason) const
{
	const BrdfTextureAsset* asset = checked_type_cast< const BrdfTextureAsset*, false >(sourceAsset);

	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getRGBAF32(), 512, 512);
	image->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	for (int32_t y = 0; y < 512; ++y)
	{
		const float fy = y / 511.0f;
		for (int32_t x = 0; x < 512; ++x)
		{
			const float fx = x / 511.0f;
			const Vector4 brdf = integrateBRDF(fx, fy);
			image->setPixel(x, y, Color4f(brdf.x(), brdf.y(), 0.0f, 0.0f));
		}
	}

	Ref< render::TextureOutput > output = new render::TextureOutput();
	output->m_textureFormat = render::TfR32G32B32A32F;
	output->m_generateMips = false;
	output->m_keepZeroAlpha = true;
	output->m_textureType = render::Tt2D;
	output->m_enableCompression = false;
	output->m_assumeLinearGamma = true;
	output->m_systemTexture = true;

	return pipelineBuilder->buildAdHocOutput(
		output,
		outputPath,
		outputGuid,
		image);
}

}
