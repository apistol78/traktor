/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <functional>
#include <embree4/rtcore.h>
#include <embree4/rtcore_ray.h>
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Memory/Alloc.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Model/Operations/MergeModel.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/GBuffer.h"
#include "Shape/Editor/Bake/IProbe.h"
#include "Shape/Editor/Bake/Embree/RayTracerEmbree.h"

#define USE_LAMBERTIAN_DIRECTION

#if defined (_MSC_VER)
#	define T_ALIGN64 __declspec(align(64))
#elif defined(__GNUC__) || defined(__ANDROID__)
#	define T_ALIGN64 __attribute__((aligned(64)))
#endif

namespace traktor::shape
{
	namespace
	{

const Scalar p(1.0f / (2.0f * PI));
const float c_epsilonOffset = 0.00001f;
const int32_t c_valid[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

class WrappedSHFunction : public render::SHFunction
{
public:
	explicit WrappedSHFunction(const std::function< Vector4 (const Vector4&) >& fn)
	:	m_fn(fn)
	{
	}

	virtual Vector4 evaluate(const Polar& direction) const override final
	{
		return m_fn(direction.toUnitCartesian());
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
	outRay.mask = -1;
	outRay.id = 0;
	outRay.flags = 0;
}

void constructRayHit(const Vector4& position, const Vector4& direction, float far, RTCRayHit& outRayHit)
{
	constructRay(position, direction, far, outRayHit.ray);
	outRayHit.hit.Ng_x = 0.0f;
	outRayHit.hit.Ng_y = 0.0f;
	outRayHit.hit.Ng_z = 0.0f;
	outRayHit.hit.u = 0.0f;
	outRayHit.hit.v = 0.0f;
	outRayHit.hit.primID = 0;
	outRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
	outRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
}

void constructRay16(const Vector4& position, const Vector4& direction, float far, int index, RTCRay16& outRay)
{
	T_ASSERT(index >= 0 && index < 16);
	outRay.org_x[index] = position.x();
	outRay.org_y[index] = position.y();
	outRay.org_z[index] = position.z();
	outRay.dir_x[index] = direction.x();
	outRay.dir_y[index] = direction.y();
	outRay.dir_z[index] = direction.z();
	outRay.tnear[index] = 0.001f;
	outRay.time[index] = 0.0f;
	outRay.tfar[index] = far;
	outRay.mask[index] = -1;
	outRay.id[index] = 0;
	outRay.flags[index] = 0;
}

void constructRayHit16(const Vector4& position, const Vector4& direction, float far, int index, RTCRayHit16& outRayHit)
{
	constructRay16(position, direction, far, index, outRayHit.ray);
	outRayHit.hit.Ng_x[index] = 0.0f;
	outRayHit.hit.Ng_y[index] = 0.0f;
	outRayHit.hit.Ng_z[index] = 0.0f;
	outRayHit.hit.u[index] = 0.0f;
	outRayHit.hit.v[index] = 0.0f;
	outRayHit.hit.primID[index] = 0;
	outRayHit.hit.geomID[index] = RTC_INVALID_GEOMETRY_ID;
	outRayHit.hit.instID[0][index] = RTC_INVALID_GEOMETRY_ID;
}

Vector4 getHitNormal(const RTCRayHit& rayHit)
{
	return Vector4::loadAligned(&rayHit.hit.Ng_x).xyz0().normalized();
}

Vector4 getHitNormal(const RTCRayHit16& rayHit, int index)
{
	return Vector4(rayHit.hit.Ng_x[index], rayHit.hit.Ng_y[index], rayHit.hit.Ng_z[index], 0.0f).normalized();
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

	m_device = rtcNewDevice(nullptr);
	m_scene = rtcNewScene(m_device);
	rtcSetSceneBuildQuality(m_scene, RTC_BUILD_QUALITY_HIGH);

	// Create SH sampling engine.
	m_shEngine = new render::SHEngine(3);
	m_shEngine->generateSamplePoints(100);

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
		Alloc::freeAlign(buffer);
	m_buffers.clear();
}

void RayTracerEmbree::addEnvironment(const IProbe* environment)
{
	m_environment = environment;
}

void RayTracerEmbree::addLight(const Light& light)
{
	m_lights.push_back(light);
}

void RayTracerEmbree::addModel(const model::Model* model, const Transform& transform)
{
	T_FATAL_ASSERT(model->getPolygonCount() > 0);

	// Allocate buffers with positions and texCoords.
	float* positions = (float*)Alloc::acquireAlign(3 * model->getVertices().size() * sizeof(float), 16, T_FILE_LINE);
	float* normals = (float*)Alloc::acquireAlign(3 * model->getVertices().size() * sizeof(float), 16, T_FILE_LINE);
	float* texCoords = (float*)Alloc::acquireAlign(2 * model->getVertices().size() * sizeof(float), 16, T_FILE_LINE);

	m_buffers.push_back(positions);
	m_buffers.push_back(normals);
	m_buffers.push_back(texCoords);

	// Copy positions, normals and texCoords.
	float* pp = positions;
	float* pn = normals;
	float* pt = texCoords;
	for (uint32_t i = 0; i < model->getVertexCount(); ++i)
	{
		const auto& vertex = model->getVertex(i);
		T_FATAL_ASSERT(vertex.getNormal() != model::c_InvalidIndex);

		const Vector4 p = transform * model->getPosition(vertex.getPosition()).xyz1();
		*pp++ = p.x();
		*pp++ = p.y();
		*pp++ = p.z();

		const Vector4 n = transform.rotation() * model->getNormal(vertex.getNormal()).xyz0();
		*pn++ = n.x();
		*pn++ = n.y();
		*pn++ = n.z();

		const Vector2 uv = (vertex.getTexCoord(0) != model::c_InvalidIndex) ? model->getTexCoord(vertex.getTexCoord(0)) : Vector2::zero();
		*pt++ = uv.x;
		*pt++ = uv.y;

		m_boundingBox.contain(p);
	}

	RTCGeometry mesh = rtcNewGeometry(m_device, RTC_GEOMETRY_TYPE_TRIANGLE);
	rtcSetGeometryVertexAttributeCount(mesh, 2);

	rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, positions, 0, 3 * sizeof(float), model->getVertices().size());
	rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3, normals, 0, 3 * sizeof(float), model->getVertices().size());
	rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, RTC_FORMAT_FLOAT2, texCoords, 0, 2 * sizeof(float), model->getVertices().size());

	uint32_t* triangles = (uint32_t*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(uint32_t), model->getPolygons().size());
	for (const auto& polygon : model->getPolygons())
	{
		T_FATAL_ASSERT(polygon.getVertexCount() == 3);
		*triangles++ = polygon.getVertex(2);
		*triangles++ = polygon.getVertex(1);
		*triangles++ = polygon.getVertex(0);
	}

	// Add filter functions if model contain alpha-test material.
	for (const auto& material : model->getMaterials())
	{
		//if (
		//	material.getBlendOperator() == model::Material::BoAlphaTest &&
		//	material.getDiffuseMap().image != nullptr
		//)
		//{
		//	rtcSetGeometryOccludedFilterFunction(mesh, alphaTestFilter);
		//	rtcSetGeometryIntersectFilterFunction(mesh, alphaTestFilter);
		//}

		if (material.getBlendOperator() != model::Material::BoDecal)
			rtcSetGeometryOccludedFilterFunction(mesh, shadowOccluded);
	}

	// Attach this class as user data to geometry.
	rtcSetGeometryUserData(mesh, this);

	rtcCommitGeometry(mesh);
	const uint32_t geomID = rtcAttachGeometry(m_scene, mesh);
	rtcReleaseGeometry(mesh);

	// Create a flatten list of materials to reduce number of indirections while tracing.
	m_materialOffset.push_back((uint32_t)m_materials.size());
	for (const auto& polygon : model->getPolygons())
	{
		const auto& material = model->getMaterial(polygon.getMaterial());
		m_materials.push_back(&material);
	}

	m_models.push_back(model);
}

void RayTracerEmbree::commit()
{
	rtcCommitScene(m_scene);

	const RTCError error = rtcGetDeviceError(m_device);
	T_FATAL_ASSERT(error == RTC_ERROR_NONE);
}

Ref< render::SHCoeffs > RayTracerEmbree::traceProbe(const Vector4& position) const
{
	static thread_local RandomGeometry random;
	static const float ProbeSize = 4.0f;

	WrappedSHFunction shFunction([&] (const Vector4& unit) -> Vector4 {

		RTCRayHit T_ALIGN64 rh;
		constructRayHit(position, unit, ProbeSize, rh);

		RTCIntersectArguments iargs;
		rtcInitIntersectArguments(&iargs);
		iargs.feature_mask = (RTCFeatureFlags)RTC_FEATURE_FLAG_TRIANGLE;
		rtcIntersect1(m_scene, &rh, &iargs);

		if (rh.hit.geomID != RTC_INVALID_GEOMETRY_ID)
		{
			float T_MATH_ALIGN16 normal[4];
			const RTCGeometry geometry = rtcGetGeometry(m_scene, rh.hit.geomID);
			rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, normal, 3);
			const Vector4 hitNormal = Vector4::loadAligned(normal).xyz0().normalized();
			if (dot3(hitNormal, unit) > 0.0f)
			{
				// Probe most likely inside geometry; offset position.
				return tracePath0(position + unit * Scalar(ProbeSize), unit, random, 0);
			}
		}

		return tracePath0(position + unit * 0.1_simd, unit, random, 0);
	});

	Ref< render::SHCoeffs > shCoeffs = new render::SHCoeffs();
	m_shEngine->generateCoefficients(&shFunction, false, *shCoeffs);
	return shCoeffs;
}

void RayTracerEmbree::traceLightmap(const model::Model* model, const GBuffer* gbuffer, drawing::Image* lightmapDiffuse, const int32_t region[4]) const
{
	RandomGeometry random;

	const Scalar ambientOcclusion(m_configuration->getAmbientOcclusionFactor());

	const auto& polygons = model->getPolygons();
	const auto& materials = model->getMaterials();

	for (int32_t y = region[1]; y < region[3]; ++y)
	{
		for (int32_t x = region[0]; x < region[2]; ++x)
		{
			const auto& e = gbuffer->get(x, y);
			if (e.polygon == ~0U)
				continue;

			const auto& originPolygon = polygons[e.polygon];
			const auto& originMaterial = materials[originPolygon.getMaterial()];

			const Color4f emittance = originMaterial.getColor().linear() * Scalar(100.0f * originMaterial.getEmissive());

			// Trace IBL and indirect illumination.
			const Color4f incoming = tracePath0(e.position, e.normal, random, 0);

			// Trace ambient occlusion.
			Scalar occlusion = 1.0_simd;
			if (ambientOcclusion > Scalar(FUZZY_EPSILON))
				occlusion = (1.0_simd - ambientOcclusion) + ambientOcclusion * traceOcclusion(e.position, e.normal, 1.0f, random);

			// Trace sky occlusion.
			const Scalar skyOcclusion = power(traceOcclusion(e.position, Vector4(0.0f, 1.0f, 0.0f), 1000.0f, random), 0.25_simd);

			// Combine and write final lumel.
			const Color4f lightmapColor = emittance + incoming * occlusion;
			lightmapDiffuse->setPixel(x, y, lightmapColor.rgb0() + Color4f(0.0f, 0.0f, 0.0f, skyOcclusion));
		}
	}
}

Color4f RayTracerEmbree::traceRay(const Vector4& position, const Vector4& direction) const
{
	static RandomGeometry random;
	float T_MATH_ALIGN16 normal[4];

	RTCRayHit T_ALIGN64 rh;
	constructRayHit(position, direction, 10000.0_simd, rh);

	RTCIntersectArguments iargs;
	rtcInitIntersectArguments(&iargs);
	iargs.feature_mask = (RTCFeatureFlags)RTC_FEATURE_FLAG_TRIANGLE;
	rtcIntersect1(m_scene, &rh, &iargs);

	if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
	{
		// Nothing hit, sample sky if available else it's all black.
		if (m_environment)
			return m_environment->sampleRadiance(direction);
		else
			return Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	}

	const RTCGeometry geometry = rtcGetGeometry(m_scene, rh.hit.geomID);
	rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, normal, 3);

	// Get position and normal of hit.
	const Vector4 hitPosition = position + direction * Scalar(rh.ray.tfar - 0.001f); 
	const Vector4 hitNormal = Vector4::loadAligned(normal).xyz0().normalized();

	// Get material as hit.
	const uint32_t offset = m_materialOffset[rh.hit.geomID];
	const auto& hitMaterial = *m_materials[offset + rh.hit.primID];

	Color4f hitMaterialColor = hitMaterial.getColor().linear();
	const auto& image = hitMaterial.getDiffuseMap().image;
	if (image)
	{
		const uint32_t slot = 1;
		float texCoord[2] = { 0.0f, 0.0f };
		rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

		image->getPixel(
			(int32_t)(wrap(texCoord[0]) * image->getWidth()),
			(int32_t)(wrap(texCoord[1]) * image->getHeight()),
			hitMaterialColor
		);
	}

	// Calculate lighting at hit.
	const Color4f emittance = hitMaterialColor * Scalar(100.0f * hitMaterial.getEmissive());
	const Color4f BRDF = hitMaterialColor / Scalar(PI);
	const Scalar cosPhi = 1.0_simd; // clamp(-dot3(hitNormal, direction), 0.0_simd, 1.0_simd);
	const Scalar probability = 1.0_simd / Scalar(PI);
	const Color4f incoming = tracePath0(hitPosition, hitNormal, random, Light::LmDirect | Light::LmIndirect);
	const Color4f direct = sampleAnalyticalLights(
		random,
		hitPosition,
		hitNormal,
		Light::LmIndirect | Light::LmDirect,
		true
	);

	if (hitMaterial.getBlendOperator() != model::Material::BoDecal)
	{
		return traceRay(hitPosition + direction * 0.1_simd, direction);
	}

	const Color4f output =
		emittance +
		direct * hitMaterialColor +
		(incoming * BRDF * cosPhi / probability);
	return output;
}

Color4f RayTracerEmbree::tracePath0(
	const Vector4& origin,
	const Vector4& normal,
	RandomGeometry& random,
	uint32_t extraLightMask
) const
{
	constexpr int SampleBatch = 16;
	float T_MATH_ALIGN16 normalTmp[4];

	int32_t sampleCount = m_configuration->getSecondarySampleCount();
	if (sampleCount <= 0)
		return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	sampleCount = alignUp(sampleCount, SampleBatch);

	Color4f color(0.0f, 0.0f, 0.0f, 0.0f);

	RTCRayHit16 T_ALIGN64 rhv;
	Vector4 directions[SampleBatch];

	// Sample across hemisphere.
	for (int32_t i = 0; i < sampleCount; i += SampleBatch)
	{
		for (int32_t j = 0; j < SampleBatch; ++j)
		{
			const Vector2 uv = Quasirandom::hammersley(i + j, sampleCount, random);
			directions[j] = Quasirandom::uniformHemiSphere(uv, normal);
			constructRayHit16(origin, directions[j], m_configuration->getMaxPathDistance(), j, rhv);
		}

		RTCIntersectArguments iargs;
		rtcInitIntersectArguments(&iargs);
		iargs.feature_mask = (RTCFeatureFlags)RTC_FEATURE_FLAG_TRIANGLE;
		rtcIntersect16(c_valid, m_scene, &rhv, &iargs);

		for (int32_t j = 0; j < SampleBatch; ++j)
		{
			const auto& direction = directions[j];

			if (rhv.hit.geomID[j] == RTC_INVALID_GEOMETRY_ID)
			{
				// Nothing hit, sample sky if available else it's all black.
				if (m_environment)
					color += m_environment->sampleRadiance(direction);
				continue;
			}

			const RTCGeometry geometry = rtcGetGeometry(m_scene, rhv.hit.geomID[j]);

			const Scalar hitDistance = Scalar(rhv.ray.tfar[j]);
			const Vector4 hitOrigin = (origin + direction * hitDistance).xyz1();

			rtcInterpolate0(geometry, rhv.hit.primID[j], rhv.hit.u[j], rhv.hit.v[j], RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, normalTmp, 3);
			const Vector4 hitNormal = Vector4::loadAligned(normalTmp).xyz0();

			const uint32_t offset = m_materialOffset[rhv.hit.geomID[j]];
			const auto& hitMaterial = *m_materials[offset + rhv.hit.primID[j]];

			Color4f hitMaterialColor = hitMaterial.getColor().linear();
			const auto& image = hitMaterial.getDiffuseMap().image;
			if (image)
			{
				const uint32_t slot = 1;
				float texCoord[2] = { 0.0f, 0.0f };
				rtcInterpolate0(geometry, rhv.hit.primID[j], rhv.hit.u[j], rhv.hit.v[j], RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

				image->getPixel(
					(int32_t)(wrap(texCoord[0]) * image->getWidth()),
					(int32_t)(wrap(texCoord[1]) * image->getHeight()),
					hitMaterialColor
				);
				hitMaterialColor = hitMaterialColor.linear();
			}
			const Color4f emittance = hitMaterialColor * Scalar(100.0f * hitMaterial.getEmissive());
			const Color4f BRDF = hitMaterialColor; // / Scalar(PI);

			const Vector2 uv(random.nextFloat(), random.nextFloat());

#if !defined(USE_LAMBERTIAN_DIRECTION)
			const Vector4 newDirection = Quasirandom::uniformHemiSphere(uv, hitNormal);
			const Scalar probability = 1.0_simd;
#else
			const Vector4 newDirection = lambertianDirection(uv, hitNormal);
			const Scalar probability = 0.78532_simd; // 1.0_simd / Scalar(PI);	// PDF from cosine weighted direction, if uniform then this should be 1.
#endif

			// const Scalar cosPhi = dot3(newDirection, hitNormal);
			const Scalar cosPhi = abs(dot3(-direction, hitNormal));
			const Color4f incoming = traceSinglePath(hitOrigin, newDirection, m_configuration->getMaxPathDistance(), random, extraLightMask, 1);
			const Color4f direct = sampleAnalyticalLights(
				random,
				hitOrigin,
				hitNormal,
				Light::LmIndirect | extraLightMask,
				true
			);

			const Color4f output =
				emittance / hitDistance +
				direct * hitMaterialColor;
				(incoming * BRDF * cosPhi / probability);
			color += output;
		}
	}

	color /= Scalar((float)sampleCount);

	// Sample direct lighting from analytical lights.
	color += sampleAnalyticalLights(random, origin, normal, Light::LmDirect | extraLightMask, false);

	return color;
}

Color4f RayTracerEmbree::traceSinglePath(
	const Vector4& origin,
	const Vector4& direction,
	float maxDistance,
	RandomGeometry& random,
	uint32_t extraLightMask,
	int32_t depth
) const
{
	float T_MATH_ALIGN16 normalTmp[4];

	if (depth > 3 || maxDistance <= 0.0f)
		return Color4f(0.0f, 0.0f, 0.0f, 0.0f);

	RTCRayHit T_ALIGN64 rh;
	constructRayHit(origin, direction, maxDistance, rh);

	RTCIntersectArguments iargs;
	rtcInitIntersectArguments(&iargs);
	iargs.feature_mask = (RTCFeatureFlags)RTC_FEATURE_FLAG_TRIANGLE;
	rtcIntersect1(m_scene, &rh, &iargs);

	if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
	{
		// Nothing hit, sample sky if available else it's all black.
		if (m_environment)
			return m_environment->sampleRadiance(direction);
		else
			return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	}

	const RTCGeometry geometry = rtcGetGeometry(m_scene, rh.hit.geomID);

	const Scalar hitDistance = Scalar(rh.ray.tfar);
	const Vector4 hitOrigin = (origin + direction * hitDistance).xyz1();
	rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, normalTmp, 3);
	const Vector4 hitNormal = Vector4::loadAligned(normalTmp).xyz0().normalized();

	const uint32_t offset = m_materialOffset[rh.hit.geomID];
	const auto& hitMaterial = *m_materials[offset + rh.hit.primID];

	Color4f hitMaterialColor = hitMaterial.getColor().linear();
	const auto& image = hitMaterial.getDiffuseMap().image;
	if (image)
	{
		const uint32_t slot = 1;
		float texCoord[2] = { 0.0f, 0.0f };
		rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

		image->getPixel(
			(int32_t)(wrap(texCoord[0]) * image->getWidth()),
			(int32_t)(wrap(texCoord[1]) * image->getHeight()),
			hitMaterialColor
		);
		hitMaterialColor = hitMaterialColor.linear();
	}
	const Color4f emittance = hitMaterialColor * Scalar(100.0f * hitMaterial.getEmissive());
	const Color4f BRDF = hitMaterialColor; // / Scalar(PI);

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
	const Color4f incoming = traceSinglePath(hitOrigin, newDirection, maxDistance - hitDistance, random, extraLightMask, depth + 1);
	const Color4f direct = sampleAnalyticalLights(
		random,
		hitOrigin,
		hitNormal,
		Light::LmIndirect | extraLightMask,
		true
	);

	const Color4f output = emittance / hitDistance + (incoming * BRDF * cosPhi / probability) + direct * hitMaterialColor; // * cosPhi;
	return output;

	//const Color4f incoming = direct + traceSinglePath(hitOrigin, newDirection, maxDistance - hitDistance, random, extraLightMask, depth + 1);
	//const Color4f color = incoming * dot3(hitNormal, -direction) * hitMaterialColor;
	//return color;
}

Scalar RayTracerEmbree::traceOcclusion(
	const Vector4& origin,
	const Vector4& normal,
	float maxDistance,
	RandomGeometry& random
) const
{
	const int32_t sampleCount = alignUp(m_configuration->getShadowSampleCount(), 16);
	RTCRay16 T_ALIGN64 rv;
	int32_t unoccluded = 0;

	for (int32_t i = 0; i < sampleCount; i += 16)
	{
		for (int32_t j = 0; j < 16; ++j)
		{
			const Vector2 uv = Quasirandom::hammersley(i + j, sampleCount, random);
			const Vector4 direction = Quasirandom::uniformHemiSphere(uv, normal);
			T_FATAL_ASSERT(dot3(direction, normal) >= 0.0_simd);
			constructRay16(origin, direction, maxDistance, j, rv);
		}

		// Intersect test all rays using ray streams.
		RTCOccludedArguments oargs;
		rtcInitOccludedArguments(&oargs);
		oargs.feature_mask = (RTCFeatureFlags)RTC_FEATURE_FLAG_TRIANGLE;
		rtcOccluded16(c_valid, m_scene, &rv, &oargs);

		// Count number of occluded rays.
		for (int32_t j = 0; j < 16; ++j)
		{
			if (rv.tfar[j] > rv.tnear[j])
				unoccluded++;
		}
	}

	return Scalar(float(unoccluded) / sampleCount);
}

Color4f RayTracerEmbree::sampleAnalyticalLights(
	RandomGeometry& random,
	const Vector4& origin,
	const Vector4& normal,
	uint8_t mask,
	bool bounce
 ) const
{
	const uint32_t shadowSampleCount = !bounce ? (uint32_t)m_shadowSampleOffsets.size() : (m_shadowSampleOffsets.size() > 0 ? 1 : 0);
	const float shadowRadius = !bounce ? m_configuration->getPointLightShadowRadius() : 0.0f;
	const Scalar lightAttenution = Scalar(m_configuration->getAnalyticalLightAttenuation());
	RTCRay T_ALIGN64 r = {};

	Color4f contribution(0.0f, 0.0f, 0.0f, 0.0f);
	for (const auto& light : m_lights)
	{
		if ((light.mask & mask) == 0)
			continue;

		switch (light.type)
		{
		case Light::LtDirectional:
			{
				const Scalar phi = dot3(normal, -light.direction);
				if (phi <= 0.0f)
					break;

				Scalar shadowAttenuate = 1.0_simd;

				if (shadowSampleCount > 0)
				{
					Vector4 u, v;
					orthogonalFrame(normal, u, v);

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
						r.tfar = 1000.0f;
						r.mask = -1;
						r.id = 0;
						r.flags = 0;
		
						RTCOccludedArguments oargs;
						rtcInitOccludedArguments(&oargs);
						oargs.feature_mask = (RTCFeatureFlags)RTC_FEATURE_FLAG_TRIANGLE;
						rtcOccluded1(m_scene, &r, &oargs);

						if (r.tfar < 0.0f)
							shadowCount++;
					}
					shadowAttenuate = Scalar(1.0f - float(shadowCount) / shadowSampleCount);
				}

				contribution += light.color * phi * shadowAttenuate * lightAttenution;
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
						r.mask = -1;
						r.id = 0;
						r.flags = 0;
		
						RTCOccludedArguments oargs;
						rtcInitOccludedArguments(&oargs);
						oargs.feature_mask = (RTCFeatureFlags)RTC_FEATURE_FLAG_TRIANGLE;
						rtcOccluded1(m_scene, &r, &oargs);

						if (r.tfar < 0.0f)
							shadowCount++;
					}
					shadowAttenuate = Scalar(1.0f - float(shadowCount) / shadowSampleCount);
				}

				contribution += light.color * phi * min(f, 1.0_simd) * shadowAttenuate * lightAttenution;
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
						r.mask = -1;
						r.id = 0;
						r.flags = 0;
		
						RTCOccludedArguments oargs;
						rtcInitOccludedArguments(&oargs);
						oargs.feature_mask = (RTCFeatureFlags)RTC_FEATURE_FLAG_TRIANGLE;
						rtcOccluded1(m_scene, &r, &oargs);

						if (r.tfar < 0.0f)
							shadowCount++;
					}
					shadowAttenuate = Scalar(1.0f - float(shadowCount) / shadowSampleCount);
				}

				contribution += light.color * k0 * k1 * k2 * shadowAttenuate * lightAttenution;
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

	for (uint32_t i = 0; i < args->N; ++i)
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
			const uint32_t slot = 1;
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

void RayTracerEmbree::shadowOccluded(const RTCFilterFunctionNArguments* args)
{
	if (args->context == nullptr)
		return;

	RayTracerEmbree* self = (RayTracerEmbree*)args->geometryUserPtr;
	RTCHitN* hits = args->hit;

	// Only rays occluded by opaque material are valid.
	for (uint32_t i = 0; i < args->N; ++i)
	{
		if (args->valid[i] != -1)
			continue;

		const uint32_t geomID = RTCHitN_geomID(hits, args->N, i);
		const uint32_t primID = RTCHitN_primID(hits, args->N, i);

		const uint32_t offset = self->m_materialOffset[geomID];
		const auto& hitMaterial = *self->m_materials[offset + primID];

		if (hitMaterial.getBlendOperator() != model::Material::BoDecal)
			args->valid[i] = 0;
	}
}

}
