/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <functional>
#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/RandomGeometry.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Model/Operations/MergeModel.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/GBuffer.h"
#include "Shape/Editor/Bake/IblProbe.h"
#include "Shape/Editor/Bake/Embree/RayTracerEmbree.h"
#include "Shape/Editor/Bake/Embree/SplitModel.h"

#define USE_LAMBERTIAN_DIRECTION

namespace traktor
{
	namespace shape
	{
		namespace
		{

const Scalar p(1.0f / (2.0f * PI));
const float c_epsilonOffset = 0.0001f;
const int32_t c_valid[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

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

Scalar attenuation(const Scalar& distance)
{
	return clamp(1.0_simd / (distance * distance), 0.0_simd, 1.0_simd);
}

Scalar attenuation(const Scalar& distance, const Scalar& range)
{
	const Scalar k0 = clamp(1.0_simd / (distance * distance), 0.0_simd, 1.0_simd);
	const Scalar k1 = clamp(1.0_simd - (distance / range), 0.0_simd, 1.0_simd);
	return k0 * k1;
}

Vector4 lambertianDirection(const Vector2& uv, const Vector4& direction)
{
	// Calculate random direction, with Gaussian probability distribution.
	const float sin2_theta = uv.x;
	const float cos2_theta = 1.0f - sin2_theta;
	const float sin_theta = std::sqrt(sin2_theta);
	const float cos_theta = std::sqrt(cos2_theta);
	const float orientation = uv.y * TWO_PI;
	const Vector4 dir(sin_theta * std::cos(orientation), sin_theta * std::sin(orientation), cos_theta, 0.0f);

	Vector4 u, v;
	orthogonalFrame(direction, u, v);
	return (Matrix44(u, v, direction, Vector4::zero()) * dir).xyz0().normalized();
}

void constructRay(const Vector4& position, const Vector4& direction, float far, RTCRay& outRay)
{
	position.storeAligned(&outRay.org_x);
	direction.storeAligned(&outRay.dir_x);
	outRay.tnear = 0.001f;
	outRay.time = 0.0f;
	outRay.tfar = far;
	outRay.mask = 0;
	outRay.id = 0;
	outRay.flags = 0;
}

void constructRay(const Vector4& position, const Vector4& direction, float far, RTCRayHit& outRayHit)
{
	constructRay(position, direction, far, outRayHit.ray);
	outRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
	outRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
}

float wrap(float n)
{
	return n - std::floor(n);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.RayTracerEmbree", 0, RayTracerEmbree, IRayTracer)

bool RayTracerEmbree::create(const BakeConfiguration* configuration)
{
	m_configuration = configuration;
	m_maxDistance = 100.0f;

	m_device = rtcNewDevice(nullptr);
	m_scene = rtcNewScene(m_device);
	rtcSetSceneBuildQuality(m_scene, RTC_BUILD_QUALITY_HIGH);

	// Create SH sampling engine.
	m_shEngine = new render::SHEngine(3);
	m_shEngine->generateSamplePoints(
		500 // configuration->getIrradianceSampleCount()
	);

	// Calculate sampling pattern of shadows, using uniform pattern within a disc.
	uint32_t sampleCount = m_configuration->getShadowSampleCount();

	// Estimate number of sample points since we "cull" points outside of circle.
	sampleCount = (uint32_t)(sampleCount * (1 + (PI * 0.25)));
	m_shadowSampleOffsets.push_back(Vector2(0.0f, 0.0f));
	for (uint32_t i = 1; i < sampleCount; ++i)
	{
		const Vector2 uv = Quasirandom::hammersley(i, sampleCount) * 2.0f - 1.0f;
		if (uv.length() <= 1.0f)
			m_shadowSampleOffsets.push_back(uv);
	}

	return true;
}

void RayTracerEmbree::destroy()
{
	m_shEngine = nullptr;
	for (auto buffer : m_buffers)
		delete[] buffer;
}

void RayTracerEmbree::addEnvironment(const IblProbe* environment)
{
	m_environment = environment;
}

void RayTracerEmbree::addLight(const Light& light)
{
	m_lights.push_back(light);
}

void RayTracerEmbree::addModel(const model::Model* model, const Transform& transform)
{
	float* positions = new float [3 * model->getVertices().size()];
	float* texCoords = new float [2 * model->getVertices().size()];

	float* pp = positions;
	float* pt = texCoords;
	for (uint32_t i = 0; i < model->getVertexCount(); ++i)
	{
		const auto& vertex = model->getVertex(i);

		const Vector4 p = transform * model->getPosition(vertex.getPosition()).xyz1();
		*pp++ = p.x();
		*pp++ = p.y();
		*pp++ = p.z();

		const Vector2 uv = (vertex.getTexCoord(0) != model::c_InvalidIndex) ? model->getTexCoord(vertex.getTexCoord(0)) : Vector2::zero();
		*pt++ = uv.x;
		*pt++ = uv.y;

		m_boundingBox.contain(p);
	}

	m_buffers.push_back(positions);
	m_buffers.push_back(texCoords);

	RefArray< const model::Model > splits;
	// splitModel(model, splits);
	splits.push_back(model);

	for (auto split : splits)
	{
		RTCGeometry mesh = rtcNewGeometry(m_device, RTC_GEOMETRY_TYPE_TRIANGLE);
		rtcSetGeometryVertexAttributeCount(mesh, 1);

		rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, positions, 0, 3 * sizeof(float), model->getVertices().size());
		rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3, texCoords, 0, 2 * sizeof(float), model->getVertices().size());

		uint32_t* triangles = (uint32_t*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(uint32_t), split->getPolygons().size());
		for (const auto& polygon : split->getPolygons())
		{
			*triangles++ = polygon.getVertex(2);
			*triangles++ = polygon.getVertex(1);
			*triangles++ = polygon.getVertex(0);
		}

		// Add filter functions if model contain alpha-test material.
		for (const auto& material : split->getMaterials())
		{
			if (
				material.getBlendOperator() == model::Material::BoAlphaTest &&
				material.getDiffuseMap().image != nullptr
			)
			{
				rtcSetGeometryOccludedFilterFunction(mesh, alphaTestFilter);
				rtcSetGeometryIntersectFilterFunction(mesh, alphaTestFilter);
			}
		}

		// Attach this class as user data to geometry.
		rtcSetGeometryUserData(mesh, this);

		rtcCommitGeometry(mesh);
		rtcAttachGeometry(m_scene, mesh);
		rtcReleaseGeometry(mesh);

		// Create a flatten list of materials to reduce number of indirections while tracing.
		m_materialOffset.push_back((uint32_t)m_materials.size());
		for (const auto& polygon : split->getPolygons())
		{
			const auto& material = model->getMaterial(polygon.getMaterial());
			m_materials.push_back(&material);
		}
	}

	m_models.push_back(model);
}

void RayTracerEmbree::commit()
{
	rtcCommitScene(m_scene);
}

Ref< render::SHCoeffs > RayTracerEmbree::traceProbe(const Vector4& position) const
{
	static thread_local RandomGeometry random;

	WrappedSHFunction shFunction([&] (const Vector4& unit) -> Vector4 {
		return tracePath0(position, unit, random);
	});

	Ref< render::SHCoeffs > shCoeffs = new render::SHCoeffs();
	m_shEngine->generateCoefficients(&shFunction, false, *shCoeffs);
	return shCoeffs;
}

void RayTracerEmbree::traceLightmap(const model::Model* model, const GBuffer* gbuffer, drawing::Image* lightmapDiffuse, const int32_t region[4]) const
{
	RTCRayHit T_MATH_ALIGN16 rh;
	RandomGeometry random;

	const int32_t sampleCount = m_configuration->getPrimarySampleCount();
	const Scalar ambientOcclusion(m_configuration->getAmbientOcclusionFactor());

	const auto& polygons = model->getPolygons();
	const auto& materials = model->getMaterials();

	for (int32_t y = region[1]; y < region[3]; ++y)
	{
		for (int32_t x = region[0]; x < region[2]; ++x)
		{
			auto elm = gbuffer->get(x, y);
			if (elm.polygon == model::c_InvalidIndex)
				continue;

			// Adjust gbuffer position to reduce shadowing issues.
			{
				const Scalar l = Scalar(elm.delta);
				const Scalar hl = l * 1.0_simd;

				const Vector4 normal = elm.normal;
				Vector4 position = elm.position + normal * hl;

				Vector4 u, v;
				orthogonalFrame(normal, u, v);

				for (int32_t i = 0; i < 16; ++i)
				{
					const float a = TWO_PI * i / 16.0f;
					const float s = sin(a), c = cos(a);

					const Vector4 traceDirection = (u * Scalar(c) + v * Scalar(s)).normalized();
					constructRay(position, traceDirection, hl, rh);

					RTCIntersectContext context;
					rtcInitIntersectContext(&context);
					rtcIntersect1(m_scene, &context, &rh);					

					if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
						continue;

					const Vector4 hitNormal = Vector4::loadAligned(&rh.hit.Ng_x).xyz0().normalized();
					if (dot3(hitNormal, traceDirection) < 0.0f)
						continue;

					// Offset position.
					position += traceDirection * Scalar(rh.ray.tfar - 0.001f) + hitNormal * 0.001_simd;
				}

				elm.position = position;
			}

			// Trace lightmap.
			{
				const auto& originPolygon = polygons[elm.polygon];
				const auto& originMaterial = materials[originPolygon.getMaterial()];

				const Color4f emittance = originMaterial.getColor() * Scalar(100.0f * originMaterial.getEmissive());

				// Trace IBL and indirect illumination.
				const Color4f incoming = tracePath0(elm.position, elm.normal, random);

				// Trace ambient occlusion.
				Scalar occlusion = 1.0_simd;
				if (ambientOcclusion > Scalar(FUZZY_EPSILON))
					occlusion = (1.0_simd - ambientOcclusion) + ambientOcclusion * traceAmbientOcclusion(elm.position, elm.normal, random);

				// Combine and write final lumel.
				lightmapDiffuse->setPixel(x, y, (emittance + incoming * occlusion).rgb1());
			}
		}
	}
}

Color4f RayTracerEmbree::traceRay(const Vector4& position, const Vector4& direction) const
{
	RTCRayHit T_MATH_ALIGN16 rh;
	constructRay(position, direction, 10000.0_simd, rh);

	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(m_scene, &context, &rh);	

	if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
	{
		// Nothing hit, sample sky if available else it's all black.
		if (m_environment)
			return m_environment->sampleRadiance(direction);
		else
			return Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	}

	const Vector4 hitPosition = position + direction * Scalar(rh.ray.tfar - 0.001f); 
	const Vector4 hitNormal = Vector4::loadAligned(&rh.hit.Ng_x).xyz0().normalized();

	const uint32_t offset = m_materialOffset[rh.hit.geomID];
	const auto& hitMaterial = *m_materials[offset + rh.hit.primID];

	Color4f hitMaterialColor = hitMaterial.getColor();
	const auto& image = hitMaterial.getDiffuseMap().image;
	if (image)
	{
		const uint32_t slot = 0;
		float texCoord[2] = { 0.0f, 0.0f };

		RTCGeometry geometry = rtcGetGeometry(m_scene, rh.hit.geomID);
		rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

		image->getPixel(
			(int32_t)(wrap(texCoord[0]) * image->getWidth()),
			(int32_t)(wrap(texCoord[1]) * image->getHeight()),
			hitMaterialColor
		);
	}
	const Color4f emittance = hitMaterialColor * Scalar(100.0f * hitMaterial.getEmissive());
	const Color4f BRDF = hitMaterialColor / Scalar(PI);
	const Scalar cosPhi = clamp(-dot3(hitNormal, direction), 0.0_simd, 1.0_simd);
	const Scalar probability = 1.0_simd / Scalar(PI);

	static RandomGeometry random;
	const Color4f incoming = tracePath0(hitPosition, hitNormal, random);

	const Color4f output = emittance + (incoming * BRDF * cosPhi / probability);
	return output;
}

Color4f RayTracerEmbree::tracePath0(
	const Vector4& origin,
	const Vector4& normal,
	RandomGeometry& random
) const
{
	constexpr int SampleBatch = 32;

	int32_t sampleCount = m_configuration->getSecondarySampleCount();
	if (sampleCount <= 0)
		return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	sampleCount = alignUp(sampleCount, SampleBatch);

	Color4f color(0.0f, 0.0f, 0.0f, 0.0f);

#if 0
	const Color4f BRDF = Color4f(1.0f, 1.0f, 1.0f, 1.0f) / Scalar(PI);
	const Scalar probability = 1.0_simd / (2.0_simd * Scalar(PI));

	// Sample across hemisphere.
	for (int32_t i = 0; i < sampleCount; ++i)
	{
		const Vector2 uv = Quasirandom::hammersley(i, sampleCount, random);
		const Vector4 direction = Quasirandom::uniformHemiSphere(uv, normal);
		const Scalar cosPhi = dot3(direction, normal);
		const Color4f incoming = traceSinglePath(origin, direction, random, 1);
		color += incoming * BRDF * cosPhi / probability;
	}
#else
	RTCRayHit T_MATH_ALIGN16 rhv[SampleBatch];
	Vector4 directions[SampleBatch];

	RTCIntersectContext context;
	rtcInitIntersectContext(&context);

	// Sample across hemisphere.
	for (int32_t i = 0; i < sampleCount; i += SampleBatch)
	{
		for (int32_t j = 0; j < SampleBatch; ++j)
		{
			const Vector2 uv = Quasirandom::hammersley(i + j, sampleCount, random);
			directions[j] = Quasirandom::uniformHemiSphere(uv, normal);
			constructRay(origin, directions[j], m_configuration->getMaxPathDistance(), rhv[j]);
		}

		rtcIntersect1M(m_scene, &context, rhv, SampleBatch, sizeof(RTCRayHit));

		for (int32_t j = 0; j < SampleBatch; ++j)
		{
			const auto& rh = rhv[j];
			const auto& direction = directions[j];

			if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
			{
				// Nothing hit, sample sky if available else it's all black.
				if (m_environment)
					color += m_environment->sampleRadiance(direction);
				continue;
			}

			const Vector4 hitNormal = Vector4::loadAligned(&rh.hit.Ng_x).xyz0().normalized();
			const Vector4 hitOrigin = (origin + direction * Scalar(rh.ray.tfar)).xyz1();

			const uint32_t offset = m_materialOffset[rh.hit.geomID];
			const auto& hitMaterial = *m_materials[offset + rh.hit.primID];

			Color4f hitMaterialColor = hitMaterial.getColor();
			const auto& image = hitMaterial.getDiffuseMap().image;
			if (image)
			{
				const uint32_t slot = 0;
				float texCoord[2] = { 0.0f, 0.0f };

				RTCGeometry geometry = rtcGetGeometry(m_scene, rh.hit.geomID);
				rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

				image->getPixel(
					(int32_t)(wrap(texCoord[0]) * image->getWidth()),
					(int32_t)(wrap(texCoord[1]) * image->getHeight()),
					hitMaterialColor
				);
			}
			const Color4f emittance = hitMaterialColor * Scalar(100.0f * hitMaterial.getEmissive());
			const Color4f BRDF = hitMaterialColor / Scalar(PI);

			const Vector2 uv(random.nextFloat(), random.nextFloat());

#if !defined(USE_LAMBERTIAN_DIRECTION)
			const Vector4 newDirection = Quasirandom::uniformHemiSphere(uv, hitNormal);
			const Scalar probability = 1.0_simd;
#else
			const Vector4 newDirection = lambertianDirection(uv, hitNormal);
			const Scalar probability = 0.78532_simd; // 1.0_simd / Scalar(PI);	// PDF from cosine weighted direction, if uniform then this should be 1.
#endif

			//const Scalar cosPhi = dot3(newDirection, hitNormal);
			const Scalar cosPhi = dot3(-direction, hitNormal);
			const Color4f incoming = traceSinglePath(hitOrigin, newDirection, random, 2);
			const Color4f direct = sampleAnalyticalLights(
				random,
				hitOrigin,
				hitNormal,
				Light::LmIndirect,
				true
			);

			const Color4f output = emittance + (incoming * BRDF * cosPhi / probability) + direct * hitMaterialColor * cosPhi;
			color += output;

			//const Color4f incoming = traceSinglePath(hitOrigin, newDirection, random, 1);
			//color += incoming * dot3(hitNormal, -direction) * hitMaterialColor;
		}
	}
#endif

	color /= Scalar(sampleCount);

	// Sample direct lighting from analytical lights.
	color += sampleAnalyticalLights(random, origin, normal, Light::LmDirect, false);

	return color;
}

Color4f RayTracerEmbree::traceSinglePath(
	const Vector4& origin,
	const Vector4& direction,
	RandomGeometry& random,
	int32_t depth
) const
{
	if (depth > 2)
		return Color4f(0.0f, 0.0f, 0.0f, 0.0f);

	RTCRayHit T_MATH_ALIGN16 rh;
	constructRay(origin, direction, m_configuration->getMaxPathDistance(), rh);

	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(m_scene, &context, &rh);

	if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
	{
		// Nothing hit, sample sky if available else it's all black.
		if (m_environment)
			return m_environment->sampleRadiance(direction);
		else
			return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	}

	const Vector4 hitNormal = Vector4::loadAligned(&rh.hit.Ng_x).xyz0().normalized();
	const Vector4 hitOrigin = (origin + direction * Scalar(rh.ray.tfar)).xyz1();

	const uint32_t offset = m_materialOffset[rh.hit.geomID];
	const auto& hitMaterial = *m_materials[offset + rh.hit.primID];

	Color4f hitMaterialColor = hitMaterial.getColor();
	const auto& image = hitMaterial.getDiffuseMap().image;
	if (image)
	{
		const uint32_t slot = 0;
		float texCoord[2] = { 0.0f, 0.0f };

		RTCGeometry geometry = rtcGetGeometry(m_scene, rh.hit.geomID);
		rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

		image->getPixel(
			(int32_t)(wrap(texCoord[0]) * image->getWidth()),
			(int32_t)(wrap(texCoord[1]) * image->getHeight()),
			hitMaterialColor
		);
	}
	const Color4f emittance = hitMaterialColor * Scalar(100.0f * hitMaterial.getEmissive());
	const Color4f BRDF = hitMaterialColor / Scalar(PI);

	const Vector2 uv(random.nextFloat(), random.nextFloat());

#if !defined(USE_LAMBERTIAN_DIRECTION)
	const Vector4 newDirection = Quasirandom::uniformHemiSphere(uv, hitNormal);
	const Scalar probability = 1.0_simd;
#else
	const Vector4 newDirection = lambertianDirection(uv, hitNormal);
	const Scalar probability = 0.78532_simd; // 1.0_simd / Scalar(PI);	// PDF from cosine weighted direction, if uniform then this should be 1.
#endif

	//const Scalar cosPhi = dot3(newDirection, hitNormal);
	const Scalar cosPhi = dot3(-direction, hitNormal);
	const Color4f incoming = traceSinglePath(hitOrigin, newDirection, random, depth + 1);
	const Color4f direct = sampleAnalyticalLights(
		random,
		hitOrigin,
		hitNormal,
		Light::LmIndirect,
		true
	);

	const Color4f output = emittance + (incoming * BRDF * cosPhi / probability) + direct * hitMaterialColor * cosPhi;
	return output;

	//const Color4f incoming = direct + traceSinglePath(hitOrigin, newDirection, random, depth + 1);
	//const Color4f color = incoming * dot3(hitNormal, -direction) * hitMaterialColor;
	//return color;
}

Scalar RayTracerEmbree::traceAmbientOcclusion(
	const Vector4& origin,
	const Vector4& normal,
	RandomGeometry& random
) const
{
	const int32_t sampleCount = alignUp(m_configuration->getShadowSampleCount(), 16);
	const float maxOcclusionDistance = 1.0f;
	RTCRay T_ALIGN16 rv[16];
	int32_t unoccluded = 0;

	for (int32_t i = 0; i < sampleCount; i += 16)
	{
		for (int32_t j = 0; j < 16; ++j)
		{
			const Vector2 uv = Quasirandom::hammersley(i + j, sampleCount, random);
			const Vector4 direction = Quasirandom::uniformHemiSphere(uv, normal);
			constructRay(origin, direction, maxOcclusionDistance, rv[j]);
		}

		// Intersect test all rays using ray streams.
		RTCIntersectContext context;
		rtcInitIntersectContext(&context);
		context.flags = RTC_INTERSECT_CONTEXT_FLAG_INCOHERENT;
		rtcOccluded1M(m_scene, &context, rv, 16, sizeof(RTCRay));

		// Count number of occluded rays.
		for (int32_t j = 0; j < 16; ++j)
		{
			const auto& r = rv[j];
			if (r.tfar > r.tnear)
				unoccluded++;
		}
	}

	const float k = float(unoccluded) / sampleCount;
	const float o = std::pow(k, 2.0f);

	return Scalar(o);
}

Color4f RayTracerEmbree::sampleAnalyticalLights(
	RandomGeometry& random,
	const Vector4& origin,
	const Vector4& normal,
	uint8_t mask,
	bool bounce
 ) const
{
	const uint32_t shadowSampleCount = !bounce ? m_shadowSampleOffsets.size() : (m_shadowSampleOffsets.size() > 0 ? 1 : 0);
	const float shadowRadius = !bounce ? m_configuration->getPointLightShadowRadius() : 0.0f;
	RTCRay T_MATH_ALIGN16 r;

	Color4f contribution(0.0f, 0.0f, 0.0f, 0.0f);
	for (const auto& light : m_lights)
	{
		if ((light.mask & mask) == 0)
			continue;

		switch (light.type)
		{
		case Light::LtDirectional:
			{
				Scalar phi = dot3(normal, -light.direction);
				if (phi <= 0.0f)
					break;

				Scalar shadowAttenuate(1.0f);

				if (shadowSampleCount > 0)
				{
					Vector4 u, v;
					orthogonalFrame(normal, u, v);

					RTCIntersectContext context;
					rtcInitIntersectContext(&context);
					context.flags = RTC_INTERSECT_CONTEXT_FLAG_COHERENT;

					int32_t shadowCount = 0;
					for (uint32_t j = 0; j < shadowSampleCount; ++j)
					{
						const Vector4 traceDirection = -light.direction;
						const Vector2 uv = m_shadowSampleOffsets[j];
						Vector4 lumelPosition = origin;
						lumelPosition += u * Scalar(uv.x * shadowRadius) + v * Scalar(uv.y * shadowRadius);

						lumelPosition.storeAligned(&r.org_x); 
						traceDirection.storeAligned(&r.dir_x);

						r.tnear = c_epsilonOffset;
						r.time = 0.0f;
						r.tfar = m_maxDistance;

						r.mask = 0;
						r.id = 0;
						r.flags = 0;
		
						rtcOccluded1(m_scene, &context, &r);

						if (r.tfar < 0.0f)
							shadowCount++;
					}
					shadowAttenuate = Scalar(1.0f - float(shadowCount) / shadowSampleCount);
				}

				contribution += light.color * phi * shadowAttenuate;
			}
			break;

		case Light::LtPoint:
			{
				Vector4 lightDirection = (light.position - origin).xyz0();
				const Scalar lightDistance = lightDirection.normalize();
				if (lightDistance > light.range)
					break;

				const Scalar phi = dot3(normal, lightDirection);
				if (phi <= 0.0_simd)
					break;

				const Scalar f = attenuation(lightDistance, light.range);
				if (f <= 0.0_simd)
					break;

				Scalar shadowAttenuate = 1.0_simd;

				if (shadowSampleCount > 0)
				{
					Vector4 u, v;
					orthogonalFrame(lightDirection, u, v);

					RTCIntersectContext context;
					rtcInitIntersectContext(&context);
					context.flags = RTC_INTERSECT_CONTEXT_FLAG_COHERENT;

					int32_t shadowCount = 0;
					for (uint32_t j = 0; j < shadowSampleCount; ++j)
					{
						const Vector4 lumelPosition = origin;
						const Vector2 uv = m_shadowSampleOffsets[j];
						Vector4 traceDirection = (light.position - origin).xyz0().normalized();
						traceDirection = (light.position + u * Scalar(uv.x * shadowRadius) + v * Scalar(uv.y * shadowRadius) - origin).xyz0().normalized();

						lumelPosition.storeAligned(&r.org_x); 
						traceDirection.storeAligned(&r.dir_x);

						r.tnear = c_epsilonOffset;
						r.time = 0.0f;
						r.tfar = lightDistance - c_epsilonOffset * 2;

						r.mask = 0;
						r.id = 0;
						r.flags = 0;
		
						rtcOccluded1(m_scene, &context, &r);

						if (r.tfar < 0.0f)
							shadowCount++;
					}
					shadowAttenuate = Scalar(1.0f - float(shadowCount) / shadowSampleCount);
				}

				contribution += light.color * phi * min(f, 1.0_simd) * shadowAttenuate;
			}
			break;

		case Light::LtSpot:
			{
				Vector4 lightToPoint = (origin - light.position).xyz0();
				const Scalar lightDistance = lightToPoint.normalize();
				if (lightDistance > light.range)
					break;

				const float alpha = clamp< float >(dot3(light.direction, lightToPoint), -1.0f, 1.0f);
				const Scalar k0 = Scalar(1.0f - std::acos(alpha) / (light.radius / 2.0f));
				if (k0 <= 0.0_simd)
					break;

				const Scalar k1 = dot3(normal, -lightToPoint);
				if (k1 <= 0.0_simd)
					break;

				const Scalar k2 = attenuation(lightDistance, light.range);
				if (k2 <= 0.0_simd)
					break;

				Scalar shadowAttenuate = 1.0_simd;

				if (shadowSampleCount > 0)
				{
					Vector4 u, v;
					orthogonalFrame(-lightToPoint, u, v);

					RTCIntersectContext context;
					rtcInitIntersectContext(&context);
					context.flags = RTC_INTERSECT_CONTEXT_FLAG_COHERENT;

					int32_t shadowCount = 0;
					for (uint32_t j = 0; j < shadowSampleCount; ++j)
					{
						const Vector4 lumelPosition = origin;
						const Vector2 uv = m_shadowSampleOffsets[j];
						Vector4 traceDirection = (light.position - origin).xyz0().normalized();
						traceDirection = (light.position + u * Scalar(uv.x * shadowRadius) + v * Scalar(uv.y * shadowRadius) - origin).xyz0().normalized();

						lumelPosition.storeAligned(&r.org_x); 
						traceDirection.storeAligned(&r.dir_x);

						r.tnear = c_epsilonOffset;
						r.time = 0.0f;
						r.tfar = lightDistance - c_epsilonOffset * 2;

						r.mask = 0;
						r.id = 0;
						r.flags = 0;
		
						rtcOccluded1(m_scene, &context, &r);

						if (r.tfar < 0.0f)
							shadowCount++;
					}
					shadowAttenuate = Scalar(1.0f - float(shadowCount) / shadowSampleCount);
				}

				contribution += light.color * k0 * k1 * k2 * shadowAttenuate;
			}
			break;
		}
	}
	return contribution;
}

void RayTracerEmbree::alphaTestFilter(const RTCFilterFunctionNArguments* args)
{
	if (args->context == nullptr)
		return;

	RayTracerEmbree* self = (RayTracerEmbree*)args->geometryUserPtr;

	RTCHitN* hits = args->hit;
	Color4f color;

	for (int i = 0; i < args->N; ++i)
	{
		if (args->valid[i] != -1)
			continue;

		const uint32_t geomID = RTCHitN_geomID(hits, args->N, i);
		const uint32_t primID = RTCHitN_primID(hits, args->N, i);

		const uint32_t offset = self->m_materialOffset[geomID];
		const auto& hitMaterial = *self->m_materials[offset + primID];

		if (hitMaterial.getBlendOperator() != model::Material::BoAlphaTest)
			continue;

		const auto& image = hitMaterial.getDiffuseMap().image;
		if (image)
		{
			const uint32_t slot = 0;
			float texCoord[2] = { 0.0f, 0.0f };

			const float u = RTCHitN_u(hits, args->N, i);
			const float v = RTCHitN_v(hits, args->N, i);

			RTCGeometry geometry = rtcGetGeometry(self->m_scene, geomID);
			rtcInterpolate0(geometry, primID, u, v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

			if (image->getPixel(
				(int32_t)(wrap(texCoord[0]) * image->getWidth()),
				(int32_t)(wrap(texCoord[1]) * image->getHeight()),
				color
			))
			{
				if (color.getAlpha() <= 0.5_simd)
					args->valid[i] = 0;
			}
		}
	}
}

	}
}
