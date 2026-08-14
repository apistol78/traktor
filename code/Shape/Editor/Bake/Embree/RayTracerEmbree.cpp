/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Bake/Embree/RayTracerEmbree.h"

#include "Core/Containers/SmallMap.h"
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
#include "Shape/Editor/Bake/BakeOperationData.h"
#include "Shape/Editor/Bake/GBuffer.h"
#include "Shape/Editor/Bake/IProbe.h"
#include "World/WorldTypes.h"

#include <embree4/rtcore.h>
#include <embree4/rtcore_ray.h>
#include <functional>

#define USE_LAMBERTIAN_DIRECTION

#if defined(_MSC_VER)
#	define T_ALIGN64 __declspec(align(64))
#elif defined(__GNUC__) || defined(__ANDROID__)
#	define T_ALIGN64 __attribute__((aligned(64)))
#endif

namespace traktor::shape
{
namespace
{

const Scalar p(1.0f / (2.0f * PI));
const Scalar c_emissiveBoost(2.0f);
const Scalar c_lightSourceRadius(0.1f);
const float c_epsilonOffset = 0.00001f;
alignas(64) const int32_t c_valid[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
const RTCFeatureFlags c_featureFlags = (RTCFeatureFlags)(RTC_FEATURE_FLAG_TRIANGLE | RTC_FEATURE_FLAG_INSTANCE);

class WrappedSHFunction : public render::SHFunction
{
public:
	explicit WrappedSHFunction(const std::function< Vector4(const Vector4&) >& fn)
		: m_fn(fn)
	{
	}

	virtual Vector4 evaluate(const Polar& direction) const override final
	{
		return m_fn(direction.toUnitCartesian());
	}

private:
	std::function< Vector4(const Vector4&) > m_fn;
};

/*! Convert radiant flux of a punctual light into irradiance at given distance.
 *
 * Must be kept in sync with LightAttenuation in the Utilities shader module,
 * {0E3643A0-A0DA-B649-9FD8-930F4EB6D42A}, so baked and dynamic lighting agree.
 */
Scalar attenuation(const Scalar& distance)
{
	const Scalar d2 = max(distance * distance, c_lightSourceRadius * c_lightSourceRadius);
	return 1.0_simd / (4.0_simd * Scalar(PI) * d2);
}

Scalar attenuation(const Scalar& distance, const Scalar& range)
{
	const Scalar a = attenuation(distance);
	const Scalar b = clamp(1.0_simd - power(distance / range, 4.0_simd), 0.0_simd, 1.0_simd);
	return a * b * b;
}

/*! Cone of a spot light; ramp from unlit at the outer cone to fully lit at the inner.
 *
 * \param cosAngle Cosine of the angle between the axis of the light and the surface.
 * \param coneAngle Full angle of the outer cone, in radians.
 *
 * Must be kept in sync with the SpotFallOff script in the deferred lights shader,
 * {707DE0B0-0E2B-A44A-9441-9B1FCFD428AA}, and HWRT_Light_CalculateIncidentLight,
 * so baked and dynamic lighting agree.
 */
Scalar spotConeFallOff(float cosAngle, float coneAngle)
{
	const float outer = std::cos(coneAngle / 2.0f);
	const float inner = std::cos((coneAngle - deg2rad(world::c_spotLightPenumbraAngle)) / 2.0f);
	return clamp(Scalar((cosAngle - outer) / (inner - outer)), 0.0_simd, 1.0_simd);
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

// Note; embree reports Ng in the space of the instanced geometry, so a hit on an
// instance need to be rotated by the instance transform to become world space.
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

bool RayTracerEmbree::create(const BakeOperationData* configuration)
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

	for (auto geometry : m_geometries)
	{
		if (geometry->mesh != nullptr)
			rtcReleaseGeometry(geometry->mesh);
		if (geometry->scene != nullptr)
			rtcReleaseScene(geometry->scene);
		for (auto buffer : geometry->buffers)
			Alloc::freeAlign(buffer);
		delete geometry;
	}
	m_geometries.clear();
	m_instances.clear();
	m_placements.clear();

	if (m_scene != nullptr)
	{
		rtcReleaseScene(m_scene);
		m_scene = nullptr;
	}
	if (m_device != nullptr)
	{
		rtcReleaseDevice(m_device);
		m_device = nullptr;
	}
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
	if (model->getPolygonCount() == 0)
		return;

	// Only record the placement; the actual geometry cannot be created until all
	// placements are known since that determines if a model should be instanced.
	m_placements.push_back({ model, transform });
}

void RayTracerEmbree::commit()
{
	// Count number of placements per model. A model placed more than once is built
	// into a scene of its own which is then instanced for each placement, so both
	// vertex buffers and BVH are shared. A model placed only once has nothing to
	// share, thus it's baked into world space and attached directly to the top
	// level scene to avoid paying for the instancing indirection while tracing.
	SmallMap< const model::Model*, uint32_t > placementCounts;
	for (const auto& placement : m_placements)
		placementCounts[placement.model]++;

	SmallMap< const model::Model*, Geometry* > instancedGeometries;

	m_instances.resize(m_placements.size());

	for (const auto& placement : m_placements)
	{
		Instance instance;
		uint32_t geometryID;

		if (placementCounts[placement.model] <= 1)
		{
			instance.geometry = createGeometry(placement.model, placement.transform);
			geometryID = rtcAttachGeometry(m_scene, instance.geometry->mesh);

			m_boundingBox.contain(instance.geometry->boundingBox);
		}
		else
		{
			Geometry*& geometry = instancedGeometries[placement.model];
			if (geometry == nullptr)
			{
				geometry = createGeometry(placement.model, Transform::identity());
				geometry->scene = rtcNewScene(m_device);
				rtcSetSceneBuildQuality(geometry->scene, RTC_BUILD_QUALITY_HIGH);
				rtcAttachGeometry(geometry->scene, geometry->mesh);
				rtcCommitScene(geometry->scene);
			}

			// Our transforms are rigid so the affine 3x4 matrix expected by embree is
			// simply the rotated basis vectors followed by the translation.
			const Matrix44 m = placement.transform.toMatrix44();
			float xfm[12];
			m.axisX().storeUnaligned3(&xfm[0]);
			m.axisY().storeUnaligned3(&xfm[3]);
			m.axisZ().storeUnaligned3(&xfm[6]);
			m.translation().storeUnaligned3(&xfm[9]);

			RTCGeometry instanceMesh = rtcNewGeometry(m_device, RTC_GEOMETRY_TYPE_INSTANCE);
			rtcSetGeometryInstancedScene(instanceMesh, geometry->scene);
			rtcSetGeometryTransform(instanceMesh, 0, RTC_FORMAT_FLOAT3X4_COLUMN_MAJOR, xfm);
			rtcCommitGeometry(instanceMesh);

			geometryID = rtcAttachGeometry(m_scene, instanceMesh);
			rtcReleaseGeometry(instanceMesh);

			instance.geometry = geometry;
			instance.rotation = placement.transform.rotation();

			m_boundingBox.contain(geometry->boundingBox.transform(placement.transform));
		}

		if (geometryID >= m_instances.size())
			m_instances.resize(geometryID + 1);
		m_instances[geometryID] = instance;
	}

	log::info << L"Ray trace world; " << (uint32_t)m_placements.size() << L" placement(s) of " << (uint32_t)m_geometries.size() << L" unique model(s), " << (uint32_t)instancedGeometries.size() << L" instanced." << Endl;

	m_placements.clear();

	rtcCommitScene(m_scene);

	const RTCError error = rtcGetDeviceError(m_device);
	T_FATAL_ASSERT(error == RTC_ERROR_NONE);
}

Ref< render::SHCoeffs > RayTracerEmbree::traceProbe(const Vector4& position, const Vector4& size) const
{
	static thread_local RandomGeometry random;
	static const Scalar probeSize = size.xyz0().max();

	WrappedSHFunction shFunction([&](const Vector4& unit) -> Vector4 {
		// Jitter origin within probe volume.
		const Vector4 jitteredPosition = position + size * random.nextUnit() * 0.5_simd;
		return tracePath0(jitteredPosition, unit, random, 0);
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

			const Color4f emittance = originMaterial.getColor() * c_emissiveBoost * Scalar(originMaterial.getEmissive());

			// Trace IBL and indirect illumination.
			const Color4f incoming = tracePath0(e.position, e.normal, random, 0);

			// Trace ambient occlusion.
			Scalar occlusion = 1.0_simd;
			if (ambientOcclusion > Scalar(FUZZY_EPSILON))
				occlusion = (1.0_simd - ambientOcclusion) + ambientOcclusion * traceOcclusion(e.position, e.normal, 1.0f, random);

			// Trace sky occlusion.
			// const Scalar skyOcclusion = power(traceOcclusion(e.position, Vector4(0.0f, 1.0f, 0.0f), 1000.0f, random), 0.25_simd);

			// Combine and write final lumel.
			const Color4f lightmapColor = emittance + incoming * occlusion;
			lightmapDiffuse->setPixel(x, y, lightmapColor.rgb1()); // lightmapColor.rgb0() + Color4f(0.0f, 0.0f, 0.0f, skyOcclusion));
		}
	}
}

Color4f RayTracerEmbree::traceRay(const Vector4& position, const Vector4& direction) const
{
	static RandomGeometry random;
	float T_MATH_ALIGN16 normal[4];
	float T_MATH_ALIGN16 color[4];

	RTCRayHit T_ALIGN64 rh;
	constructRayHit(position, direction, 10000.0_simd, rh);

	RTCIntersectArguments iargs;
	rtcInitIntersectArguments(&iargs);
	iargs.feature_mask = c_featureFlags;
	rtcIntersect1(m_scene, &rh, &iargs);

	if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
	{
		// Nothing hit, sample sky if available else it's all black.
		if (m_environment)
			return m_environment->sampleRadiance(direction);
		else
			return Color4f(0.0f, 0.0f, 0.0f, 1.0f);
	}

	const Instance& instance = getInstance(rh.hit.instID[0], rh.hit.geomID);
	const RTCGeometry geometry = instance.geometry->mesh;
	rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, normal, 3);
	rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 2, color, 4);

	// Get position and normal of hit; interpolated attributes are in the space of
	// the instanced geometry so the normal need to be rotated into world space.
	const Vector4 hitPosition = position + direction * Scalar(rh.ray.tfar - 0.001f);
	const Vector4 hitNormal = (instance.rotation * Vector4::loadAligned(normal).xyz0()).normalized();
	const Vector4 hitColor = Vector4::loadAligned(color);

	// Get material as hit.
	const auto& hitMaterial = *instance.geometry->materials[rh.hit.primID];

	Color4f hitMaterialColor = lerp(hitMaterial.getColor(), Color4f(hitColor), hitColor.w());

	const auto& image = hitMaterial.getDiffuseMap().image;
	if (image)
	{
		const uint32_t slot = 1;
		float texCoord[2] = { 0.0f, 0.0f };
		rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

		image->getPixel(
			(int32_t)(wrap(texCoord[0]) * image->getWidth()),
			(int32_t)(wrap(texCoord[1]) * image->getHeight()),
			hitMaterialColor);
	}

	// Calculate lighting at hit.
	const Color4f emittance = hitMaterialColor * c_emissiveBoost * Scalar(hitMaterial.getEmissive());
	const Color4f BRDF = hitMaterialColor / Scalar(PI);
	const Scalar cosPhi = 1.0_simd; // clamp(-dot3(hitNormal, direction), 0.0_simd, 1.0_simd);
	const Scalar probability = 1.0_simd / Scalar(PI);
	const Color4f incoming = tracePath0(hitPosition, hitNormal, random, Light::LmDirect | Light::LmIndirect);
	const Color4f direct = sampleAnalyticalLights(
		random,
		hitPosition,
		hitNormal,
		Light::LmIndirect | Light::LmDirect,
		true);

	if (hitMaterial.getBlendOperator() != model::Material::BoDecal)
		return traceRay(hitPosition + direction * 0.1_simd, direction);

	const Color4f output =
		emittance +
		((incoming + direct) * BRDF * cosPhi / probability);
	return output;
}

RayTracerEmbree::Geometry* RayTracerEmbree::createGeometry(const model::Model* model, const Transform& transform)
{
	const uint32_t vertexCount = model->getVertexCount();

	Geometry* geometry = new Geometry();
	geometry->model = model;

	// Allocate buffers with positions and texCoords.
	float* positions = (float*)Alloc::acquireAlign(vertexCount * 3 * sizeof(float) + 16, 16, T_FILE_LINE);
	float* normals = (float*)Alloc::acquireAlign(vertexCount * 3 * sizeof(float) + 16, 16, T_FILE_LINE);
	float* texCoords = (float*)Alloc::acquireAlign(vertexCount * 2 * sizeof(float) + 16, 16, T_FILE_LINE); // Allocating tuples of 3 instead of two; seems embree read outside of range.
	float* colors = (float*)Alloc::acquireAlign(vertexCount * 4 * sizeof(float) + 16, 16, T_FILE_LINE);

	geometry->buffers.push_back(positions);
	geometry->buffers.push_back(normals);
	geometry->buffers.push_back(texCoords);
	geometry->buffers.push_back(colors);

	// Copy positions, normals and texCoords.
	float* pp = positions;
	float* pn = normals;
	float* pt = texCoords;
	float* pc = colors;
	for (uint32_t i = 0; i < vertexCount; ++i)
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

		const Vector4 cl = (vertex.getColor() != model::c_InvalidIndex) ? model->getColor(vertex.getColor()).xyz1() : Vector4::zero();
		*pc++ = cl.x();
		*pc++ = cl.y();
		*pc++ = cl.z();
		*pc++ = cl.w();

		geometry->boundingBox.contain(p);
	}

	RTCGeometry mesh = rtcNewGeometry(m_device, RTC_GEOMETRY_TYPE_TRIANGLE);
	rtcSetGeometryVertexAttributeCount(mesh, 3);

	rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, positions, 0, 3 * sizeof(float), vertexCount);
	rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT3, normals, 0, 3 * sizeof(float), vertexCount);
	rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, RTC_FORMAT_FLOAT2, texCoords, 0, 2 * sizeof(float), vertexCount);
	rtcSetSharedGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 2, RTC_FORMAT_FLOAT4, colors, 0, 4 * sizeof(float), vertexCount);

	uint32_t* triangles = (uint32_t*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(uint32_t), model->getPolygons().size());
	for (const auto& polygon : model->getPolygons())
	{
		T_FATAL_ASSERT(polygon.getVertexCount() == 3);
		*triangles++ = polygon.getVertex(2);
		*triangles++ = polygon.getVertex(1);
		*triangles++ = polygon.getVertex(0);
	}

	// Create a flatten list of materials to reduce number of indirections while tracing.
	geometry->materials.reserve(model->getPolygonCount());
	for (const auto& polygon : model->getPolygons())
	{
		const auto& material = model->getMaterial(polygon.getMaterial());
		geometry->materials.push_back(&material);
	}

	// Add filter functions if model contain alpha-test material.
	for (const auto& material : model->getMaterials())
	{
		// if (
		//	material.getBlendOperator() == model::Material::BoAlphaTest &&
		//	material.getDiffuseMap().image != nullptr
		//)
		//{
		//	rtcSetGeometryOccludedFilterFunction(mesh, alphaTestFilter);
		//	rtcSetGeometryIntersectFilterFunction(mesh, alphaTestFilter);
		// }

		if (material.getBlendOperator() != model::Material::BoDecal)
			rtcSetGeometryOccludedFilterFunction(mesh, shadowOccluded);
	}

	// Attach geometry as user data; filter functions resolve materials through it.
	rtcSetGeometryUserData(mesh, geometry);

	rtcCommitGeometry(mesh);

	// Keep the handle instead of resolving it through rtcGetGeometry while tracing;
	// rtcGetGeometry isn't thread safe and we interpolate attributes from all threads.
	geometry->mesh = mesh;

	m_geometries.push_back(geometry);
	return geometry;
}

Color4f RayTracerEmbree::tracePath0(
	const Vector4& origin,
	const Vector4& normal,
	RandomGeometry& random,
	uint32_t extraLightMask) const
{
	constexpr int SampleBatch = 16;
	float T_MATH_ALIGN16 normalTmp[4];
	float T_MATH_ALIGN16 colorTmp[4];

	int32_t sampleCount = m_configuration->getSecondarySampleCount();
	if (sampleCount <= 0)
		return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	sampleCount = alignUp(sampleCount, SampleBatch);

	Color4f color(0.0f, 0.0f, 0.0f, 0.0f);

	RTCRayHit16 T_ALIGN64 rhv;
	Vector4 directions[SampleBatch];

	// One shift for the entire set of samples; decorrelate this lumel from its
	// neighbours without breaking the stratification within the set.
	const Vector2 shift(random.nextFloat(), random.nextFloat());

	// Sample across hemisphere.
	for (int32_t i = 0; i < sampleCount; i += SampleBatch)
	{
		for (int32_t j = 0; j < SampleBatch; ++j)
		{
			const Vector2 uv = Quasirandom::hammersley(i + j, sampleCount, shift);
			directions[j] = Quasirandom::uniformHemiSphere(uv, normal);
			constructRayHit16(origin, directions[j], m_configuration->getMaxPathDistance(), j, rhv);
		}

		RTCIntersectArguments iargs;
		rtcInitIntersectArguments(&iargs);
		iargs.feature_mask = c_featureFlags;
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

			const Instance& instance = getInstance(rhv.hit.instID[0][j], rhv.hit.geomID[j]);
			const RTCGeometry geometry = instance.geometry->mesh;

			const Scalar hitDistance = Scalar(rhv.ray.tfar[j]);
			const Vector4 hitOrigin = (origin + direction * hitDistance).xyz1();

			rtcInterpolate0(geometry, rhv.hit.primID[j], rhv.hit.u[j], rhv.hit.v[j], RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, normalTmp, 3);
			rtcInterpolate0(geometry, rhv.hit.primID[j], rhv.hit.u[j], rhv.hit.v[j], RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 2, colorTmp, 4);

			// Interpolated attributes are in the space of the instanced geometry
			// so the normal need to be rotated into world space.
			const Vector4 hitNormal = instance.rotation * Vector4::loadAligned(normalTmp).xyz0();
			const Vector4 hitColor = Vector4::loadAligned(colorTmp);

			const auto& hitMaterial = *instance.geometry->materials[rhv.hit.primID[j]];

			Color4f hitMaterialColor = lerp(hitMaterial.getColor(), Color4f(hitColor), hitColor.w());

			const auto& image = hitMaterial.getDiffuseMap().image;
			if (image)
			{
				const uint32_t slot = 1;
				float texCoord[2] = { 0.0f, 0.0f };
				rtcInterpolate0(geometry, rhv.hit.primID[j], rhv.hit.u[j], rhv.hit.v[j], RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

				image->getPixel(
					(int32_t)(wrap(texCoord[0]) * image->getWidth()),
					(int32_t)(wrap(texCoord[1]) * image->getHeight()),
					hitMaterialColor);
			}
			const Color4f emittance = hitMaterialColor * c_emissiveBoost * Scalar(hitMaterial.getEmissive());

			const Vector2 uv(random.nextFloat(), random.nextFloat());

#if !defined(USE_LAMBERTIAN_DIRECTION)
			// Uniformly distributed direction; pdf is 1/2pi so the estimate of the
			// irradiance integral need the cosine at the hit, and 2pi from the pdf
			// cancelled against the 1/pi of the lambertian BRDF.
			const Vector4 newDirection = Quasirandom::uniformHemiSphere(uv, hitNormal);
			const Scalar weight = 2.0_simd * clamp(dot3(newDirection, hitNormal), 0.0_simd, 1.0_simd);
#else
			// Cosine distributed direction; pdf is cos/pi which cancel both the
			// cosine and the 1/pi of the lambertian BRDF exactly.
			const Vector4 newDirection = Quasirandom::lambertian(uv, hitNormal);
			const Scalar weight = 1.0_simd;
#endif

			const Color4f incoming = traceSinglePath(hitOrigin, newDirection, m_configuration->getMaxPathDistance(), random, extraLightMask, 1);
			const Color4f direct = sampleAnalyticalLights(
				random,
				hitOrigin,
				hitNormal,
				Light::LmIndirect | extraLightMask,
				true);

			// Radiance leaving the hit surface towards our lumel; direct is irradiance
			// so it need the 1/pi of the lambertian BRDF, indirect already got it folded
			// into the sample weight. Emittance is radiance, invariant along the ray.
			const Color4f radiance = emittance + hitMaterialColor * (direct / Scalar(PI) + incoming * weight);

			color += radiance * clamp(dot3(direction, normal), 0.0_simd, 1.0_simd);
		}
	}

	// Hemisphere is uniformly sampled, pdf 1/2pi, so irradiance is (2pi/N) * sum;
	// we accumulate irradiance divided by pi, as expected by the shaders, hence 2/N.
	color = (color * 2.0_simd) / Scalar((float)sampleCount);

	// Sample direct lighting from analytical lights; irradiance, so divide by pi as above.
	color += sampleAnalyticalLights(random, origin, normal, Light::LmDirect | extraLightMask, false) / Scalar(PI);

	return color;
}

Color4f RayTracerEmbree::traceSinglePath(
	const Vector4& origin,
	const Vector4& direction,
	float maxDistance,
	RandomGeometry& random,
	uint32_t extraLightMask,
	int32_t depth) const
{
	float T_MATH_ALIGN16 normalTmp[4];
	float T_MATH_ALIGN16 colorTmp[4];

	if (depth > 3 || maxDistance <= 0.0f)
		return Color4f(0.0f, 0.0f, 0.0f, 0.0f);

	RTCRayHit T_ALIGN64 rh;
	constructRayHit(origin, direction, maxDistance, rh);

	RTCIntersectArguments iargs;
	rtcInitIntersectArguments(&iargs);
	iargs.feature_mask = c_featureFlags;
	rtcIntersect1(m_scene, &rh, &iargs);

	if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
	{
		// Nothing hit, sample sky if available else it's all black.
		if (m_environment)
			return m_environment->sampleRadiance(direction);
		else
			return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	}

	const Instance& instance = getInstance(rh.hit.instID[0], rh.hit.geomID);
	const RTCGeometry geometry = instance.geometry->mesh;

	const Scalar hitDistance = Scalar(rh.ray.tfar);
	const Vector4 hitOrigin = (origin + direction * hitDistance).xyz1();

	rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, normalTmp, 3);
	rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 2, colorTmp, 4);

	// Interpolated attributes are in the space of the instanced geometry so the
	// normal need to be rotated into world space.
	const Vector4 hitNormal = (instance.rotation * Vector4::loadAligned(normalTmp).xyz0()).normalized();
	const Vector4 hitColor = Vector4::loadAligned(colorTmp);

	const auto& hitMaterial = *instance.geometry->materials[rh.hit.primID];

	Color4f hitMaterialColor = lerp(hitMaterial.getColor(), Color4f(hitColor), hitColor.w());

	const auto& image = hitMaterial.getDiffuseMap().image;
	if (image)
	{
		const uint32_t slot = 1;
		float texCoord[2] = { 0.0f, 0.0f };
		rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

		image->getPixel(
			(int32_t)(wrap(texCoord[0]) * image->getWidth()),
			(int32_t)(wrap(texCoord[1]) * image->getHeight()),
			hitMaterialColor);
	}
	const Color4f emittance = hitMaterialColor * c_emissiveBoost * Scalar(hitMaterial.getEmissive());

	const Vector2 uv(random.nextFloat(), random.nextFloat());

#if !defined(USE_LAMBERTIAN_DIRECTION)
	// Uniformly distributed direction; pdf is 1/2pi so the estimate of the
	// irradiance integral need the cosine at the hit, and 2pi from the pdf
	// cancelled against the 1/pi of the lambertian BRDF.
	const Vector4 newDirection = Quasirandom::uniformHemiSphere(uv, hitNormal);
	const Scalar weight = 2.0_simd * clamp(dot3(newDirection, hitNormal), 0.0_simd, 1.0_simd);
#else
	// Cosine distributed direction; pdf is cos/pi which cancel both the
	// cosine and the 1/pi of the lambertian BRDF exactly.
	const Vector4 newDirection = Quasirandom::lambertian(uv, hitNormal);
	const Scalar weight = 1.0_simd;
#endif

	const Color4f incoming = traceSinglePath(hitOrigin, newDirection, maxDistance - hitDistance, random, extraLightMask, depth + 1);
	const Color4f direct = sampleAnalyticalLights(
		random,
		hitOrigin,
		hitNormal,
		Light::LmIndirect | extraLightMask,
		true);

	// Radiance leaving the hit surface back along the incident ray.
	return emittance + hitMaterialColor * (direct / Scalar(PI) + incoming * weight);
}

Scalar RayTracerEmbree::traceOcclusion(
	const Vector4& origin,
	const Vector4& normal,
	float maxDistance,
	RandomGeometry& random) const
{
	const int32_t sampleCount = alignUp(m_configuration->getShadowSampleCount(), 16);
	RTCRay16 T_ALIGN64 rv;
	int32_t unoccluded = 0;

	// One shift for the entire set of samples; see tracePath0.
	const Vector2 shift(random.nextFloat(), random.nextFloat());

	for (int32_t i = 0; i < sampleCount; i += 16)
	{
		for (int32_t j = 0; j < 16; ++j)
		{
			const Vector2 uv = Quasirandom::hammersley(i + j, sampleCount, shift);
			const Vector4 direction = Quasirandom::uniformHemiSphere(uv, normal);
			T_FATAL_ASSERT(dot3(direction, normal) >= 0.0_simd);
			constructRay16(origin, direction, maxDistance, j, rv);
		}

		// Intersect test all rays using ray streams.
		RTCOccludedArguments oargs;
		rtcInitOccludedArguments(&oargs);
		oargs.feature_mask = c_featureFlags;
		rtcOccluded16(c_valid, m_scene, &rv, &oargs);

		// Count number of occluded rays.
		for (int32_t j = 0; j < 16; ++j)
			if (rv.tfar[j] > rv.tnear[j])
				unoccluded++;
	}

	return Scalar(float(unoccluded) / sampleCount);
}

Color4f RayTracerEmbree::sampleAnalyticalLights(
	RandomGeometry& random,
	const Vector4& origin,
	const Vector4& normal,
	uint8_t mask,
	bool bounce) const
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
						oargs.feature_mask = c_featureFlags;
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
						oargs.feature_mask = c_featureFlags;
						rtcOccluded1(m_scene, &r, &oargs);

						if (r.tfar < 0.0f)
							shadowCount++;
					}
					shadowAttenuate = Scalar(1.0f - float(shadowCount) / shadowSampleCount);
				}

				contribution += light.color * phi * f * shadowAttenuate * lightAttenution;
			}
			break;

		case Light::LtSpot:
			{
				Vector4 lightToPoint = (origin - light.position).xyz0();
				const Scalar lightDistance = lightToPoint.normalize();
				if (lightDistance > light.range)
					break;

				const float alpha = clamp< float >(dot3(light.direction, lightToPoint), -1.0f, 1.0f);
				const Scalar k0 = spotConeFallOff(alpha, light.radius);
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
						oargs.feature_mask = c_featureFlags;
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

	const Geometry* geometry = (const Geometry*)args->geometryUserPtr;
	RTCHitN* hits = args->hit;
	Color4f color;

	for (uint32_t i = 0; i < args->N; ++i)
	{
		if (args->valid[i] != -1)
			continue;

		const uint32_t primID = RTCHitN_primID(hits, args->N, i);
		const auto& hitMaterial = *geometry->materials[primID];

		if (hitMaterial.getBlendOperator() != model::Material::BoAlphaTest)
			continue;

		const auto& image = hitMaterial.getDiffuseMap().image;
		if (image)
		{
			const uint32_t slot = 1;
			float texCoord[2] = { 0.0f, 0.0f };

			const float u = RTCHitN_u(hits, args->N, i);
			const float v = RTCHitN_v(hits, args->N, i);

			rtcInterpolate0(geometry->mesh, primID, u, v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

			if (image->getPixel(
					(int32_t)(wrap(texCoord[0]) * image->getWidth()),
					(int32_t)(wrap(texCoord[1]) * image->getHeight()),
					color))
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

	const Geometry* geometry = (const Geometry*)args->geometryUserPtr;
	RTCHitN* hits = args->hit;

	// Only rays occluded by opaque material are valid.
	for (uint32_t i = 0; i < args->N; ++i)
	{
		if (args->valid[i] != -1)
			continue;

		const uint32_t primID = RTCHitN_primID(hits, args->N, i);
		const auto& hitMaterial = *geometry->materials[primID];

		if (hitMaterial.getBlendOperator() != model::Material::BoDecal)
			args->valid[i] = 0;
	}
}

}
