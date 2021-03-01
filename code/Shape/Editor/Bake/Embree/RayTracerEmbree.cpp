#include <functional>
#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>
#include "Core/Functor/Functor.h"
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
	return clamp(Scalar(1.0f) / (distance * distance), Scalar(0.0f), Scalar(1.0f));
}

Scalar attenuation(const Scalar& distance, const Scalar& range)
{
	Scalar k0 = clamp(Scalar(1.0f) / (distance * distance), Scalar(0.0f), Scalar(1.0f));
	Scalar k1 = clamp(Scalar(1.0f) - (distance / range), Scalar(0.0f), Scalar(1.0f));
	return k0 * k1;
}

Vector4 lambertianDirection(const Vector2& uv, const Vector4& direction)
{
	// Calculate random direction, with Gaussian probability distribution.
	float sin2_theta = uv.x;
	float cos2_theta = 1.0f - sin2_theta;
	float sin_theta = std::sqrt(sin2_theta);
	float cos_theta = std::sqrt(cos2_theta);
	float orientation = uv.y * TWO_PI;
	Vector4 dir(sin_theta * std::cos(orientation), cos_theta, sin_theta * std::sin(orientation), 0.0f);

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

RayTracerEmbree::RayTracerEmbree()
:	m_device(nullptr)
,	m_scene(nullptr)
,	m_maxDistance(0.0f)
{
}

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
		1000 // configuration->getIrradianceSampleCount()
	);

	// Calculate sampling pattern of shadows, using uniform pattern within a disc.
	uint32_t sampleCount = m_configuration->getShadowSampleCount();

	// Estimate number of sample points since we "cull" points outside of circle.
	sampleCount = (uint32_t)(sampleCount * (1 + (PI * 0.25)));
	m_shadowSampleOffsets.push_back(Vector2(0.0f, 0.0f));
	for (uint32_t i = 1; i < sampleCount; ++i)
	{
		Vector2 uv = Quasirandom::hammersley(i, sampleCount);
		uv = uv * 2.0f - 1.0f;
		if (uv.length() <= 1.0f)
			m_shadowSampleOffsets.push_back(uv);
	}

    return true;
}

void RayTracerEmbree::destroy()
{
	m_shEngine = nullptr;
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
	RTCGeometry mesh = rtcNewGeometry(m_device, RTC_GEOMETRY_TYPE_TRIANGLE);
	rtcSetGeometryVertexAttributeCount(mesh, 1);

	float* positions = (float*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), model->getVertices().size());
	float* texCoords = (float*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, RTC_FORMAT_FLOAT2, 2 * sizeof(float), model->getVertices().size());

	for (uint32_t i = 0; i < model->getVertexCount(); ++i)
	{
		const auto& vertex = model->getVertex(i);

		Vector4 p = transform * model->getPosition(vertex.getPosition()).xyz1();
		*positions++ = p.x();
		*positions++ = p.y();
		*positions++ = p.z();

		Vector2 uv(0.0f, 0.0f);
		if (vertex.getTexCoord(0) != model::c_InvalidIndex)
			uv = model->getTexCoord(vertex.getTexCoord(0));
		*texCoords++ = uv.x;
		*texCoords++ = uv.y;
	}

	uint32_t* triangles = (uint32_t*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(uint32_t), model->getPolygons().size());
	for (const auto& polygon : model->getPolygons())
	{
		*triangles++ = polygon.getVertex(2);
		*triangles++ = polygon.getVertex(1);
		*triangles++ = polygon.getVertex(0);
	}

	// Add filter functions if model contain alpha-test material.
	for (const auto& material : model->getMaterials())
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

void RayTracerEmbree::traceLightmap(const model::Model* model, const GBuffer* gbuffer, drawing::Image* lightmapDiffuse, drawing::Image* lightmapDirectional, const int32_t region[4]) const
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
				const Scalar hl = l * Scalar(1.0f);

				Vector4 normal = elm.normal;
				Vector4 position = elm.position + normal * hl;

				Vector4 u, v;
				orthogonalFrame(normal, u, v);

				for (int32_t i = 0; i < 16; ++i)
				{
					float a = TWO_PI * i / 16.0f;
					float s = sin(a), c = cos(a);

					Vector4 traceDirection = (u * Scalar(c) + v * Scalar(s)).normalized();
					constructRay(position, traceDirection, hl, rh);

					RTCIntersectContext context;
					rtcInitIntersectContext(&context);
					rtcIntersect1(m_scene, &context, &rh);					

					if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
						continue;

					Vector4 hitNormal = Vector4::loadAligned(&rh.hit.Ng_x).xyz0().normalized();
					if (dot3(hitNormal, traceDirection) < 0.0f)
						continue;

					// Offset position.
					position += traceDirection * Scalar(rh.ray.tfar - 0.001f) + hitNormal * Scalar(0.02f);
				}

				elm.position = position;
			}

			// Trace lightmap.
			Color4f incoming;
			{
				const auto& originPolygon = polygons[elm.polygon];
				const auto& originMaterial = materials[originPolygon.getMaterial()];

				Color4f emittance = originMaterial.getColor() * Scalar(100.0f * originMaterial.getEmissive());
				Scalar metalness = Scalar(originMaterial.getMetalness());

				// Trace IBL and indirect illumination.
				incoming = tracePath0(elm.position, elm.normal, random);

				// Trace ambient occlusion.
				Scalar occlusion = 1.0_simd;
				if (ambientOcclusion > Scalar(FUZZY_EPSILON))
					occlusion = (1.0_simd - ambientOcclusion) + ambientOcclusion * traceAmbientOcclusion(elm.position, elm.normal, random);

				// Combine and write final lumel.
				lightmapDiffuse->setPixel(x, y, (emittance + (incoming * occlusion) * (1.0_simd - metalness)).rgb1());
			}

			Scalar intensity = horizontalAdd3(incoming) / 3.0_simd;

			// Trace directional map.
			if (intensity > FUZZY_EPSILON)
			{
				const Scalar z(1.0f / std::sqrt(3.0f));
				const Vector4 basisX(std::sqrt(2.0f / 3.0f), 0.0f, z);
				const Vector4 basisY(-1.0f / std::sqrt(6.0f), 1.0f / std::sqrt(2.0f), z);
				const Vector4 basisZ(-1.0f / std::sqrt(6.0f), -1.0f / std::sqrt(2.0f), z);

				Vector4 binormal = -cross(elm.normal, elm.tangent);

				Matrix44 frame(
					elm.tangent,
					binormal,
					elm.normal,
					Vector4::zero()
				);

				Color4f incomingX = tracePath0(elm.position, frame * basisX, random);
				Color4f incomingY = tracePath0(elm.position, frame * basisY, random);
				Color4f incomingZ = tracePath0(elm.position, frame * basisZ, random);

				Scalar intensityX = horizontalAdd3(incomingX) / 3.0_simd;
				Scalar intensityY = horizontalAdd3(incomingY) / 3.0_simd;
				Scalar intensityZ = horizontalAdd3(incomingZ) / 3.0_simd;

				lightmapDirectional->setPixel(x, y, Color4f(
					0.5_simd * (intensityX / intensity) * (1.0_simd / z),
					0.5_simd * (intensityY / intensity) * (1.0_simd / z),
					0.5_simd * (intensityZ / intensity) * (1.0_simd / z),
					1.0f
				));
			}
		}
	}
}

Color4f RayTracerEmbree::tracePath0(
	const Vector4& origin,
	const Vector4& normal,
	RandomGeometry& random
) const
{
	int32_t sampleCount = m_configuration->getSecondarySampleCount();
	if (sampleCount <= 0)
		return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	sampleCount = alignUp(sampleCount, 16);

	Color4f color(0.0f, 0.0f, 0.0f, 0.0f);

#if 0
	const Color4f BRDF = Color4f(1.0f, 1.0f, 1.0f, 1.0f) / Scalar(PI);
	const Scalar probability = 1.0_simd / (2.0_simd * Scalar(PI));

	// Sample across hemisphere.
	for (int32_t i = 0; i < sampleCount; ++i)
	{
		Vector2 uv = Quasirandom::hammersley(i, sampleCount, random);
		Vector4 direction = Quasirandom::uniformHemiSphere(uv, normal);

		Scalar cosPhi = dot3(direction, normal);

		Color4f incoming = traceSinglePath(origin, direction, random, 1);

		color += incoming * BRDF * cosPhi / probability;
	}
#else
	RTCRayHit T_MATH_ALIGN16 rhv[16];
	Vector4 directions[16];

	RTCIntersectContext context;
	rtcInitIntersectContext(&context);

	// Sample across hemisphere.
	for (int32_t i = 0; i < sampleCount; i += 16)
	{
		for (int32_t j = 0; j < 16; ++j)
		{
			Vector2 uv = Quasirandom::hammersley(i + j, sampleCount, random);
			directions[j] = Quasirandom::uniformHemiSphere(uv, normal);
			constructRay(origin, directions[j], m_configuration->getMaxPathDistance(), rhv[j]);
		}

		rtcIntersect1M(m_scene, &context, rhv, 16, sizeof(RTCRayHit));

		for (int32_t j = 0; j < 16; ++j)
		{
			const auto& rh = rhv[j];
			const auto& direction = directions[j];

			if (
				rh.hit.geomID == RTC_INVALID_GEOMETRY_ID ||
				rh.hit.geomID >= m_models.size()
			)
			{
				// Nothing hit, sample sky if available else it's all black.
				if (m_environment)
					color += m_environment->sampleRadiance(direction);
				continue;
			}

			Vector4 hitNormal = Vector4::loadAligned(&rh.hit.Ng_x).xyz0().normalized();
			Vector4 hitOrigin = (origin + direction * Scalar(rh.ray.tfar)).xyz1();

			const auto& polygons = m_models[rh.hit.geomID]->getPolygons();
			const auto& materials = m_models[rh.hit.geomID]->getMaterials();
			const auto& hitPolygon = polygons[rh.hit.primID];
			const auto& hitMaterial = materials[hitPolygon.getMaterial()];

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
			Color4f emittance = hitMaterialColor * Scalar(100.0f * hitMaterial.getEmissive());
			Color4f BRDF = hitMaterialColor / Scalar(PI);

			Vector2 uv(random.nextFloat(), random.nextFloat());

			Vector4 newDirection = Quasirandom::uniformHemiSphere(uv, hitNormal);
			const Scalar probability = 1.0_simd;

			//Vector4 newDirection = lambertianDirection(uv, hitNormal);
			//const Scalar probability = 1.0_simd / Scalar(PI);	// PDF from cosine weighted direction, if uniform then this should be 1.

			Scalar cosPhi = dot3(newDirection, hitNormal);

			Color4f incoming = traceSinglePath(hitOrigin, newDirection, random, 2);

			Color4f direct = sampleAnalyticalLights(
				random,
				hitOrigin,
				hitNormal,
				Light::LmIndirect,
				true
			);

			color += emittance + (incoming * BRDF * cosPhi / probability) + direct * hitMaterialColor * cosPhi;
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
	{
		// Terminate, sample sky if available else it's all black.
		if (m_environment)
			return m_environment->sampleRadiance(direction);
		else
			return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	}

	RTCRayHit T_MATH_ALIGN16 rh;
	constructRay(origin, direction, m_configuration->getMaxPathDistance(), rh);

	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(m_scene, &context, &rh);

	if (
		rh.hit.geomID == RTC_INVALID_GEOMETRY_ID ||
		rh.hit.geomID >= m_models.size()
	)
	{
		// Nothing hit, sample sky if available else it's all black.
		if (m_environment)
			return m_environment->sampleRadiance(direction);
		else
			return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	}

	Vector4 hitNormal = Vector4::loadAligned(&rh.hit.Ng_x).xyz0().normalized();
	Vector4 hitOrigin = (origin + direction * Scalar(rh.ray.tfar)).xyz1();

	const auto& polygons = m_models[rh.hit.geomID]->getPolygons();
	const auto& materials = m_models[rh.hit.geomID]->getMaterials();
	const auto& hitPolygon = polygons[rh.hit.primID];
	const auto& hitMaterial = materials[hitPolygon.getMaterial()];

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
	Color4f emittance = hitMaterialColor * Scalar(100.0f * hitMaterial.getEmissive());
	Color4f BRDF = hitMaterialColor / Scalar(PI);

	Vector2 uv(random.nextFloat(), random.nextFloat());

	Vector4 newDirection = Quasirandom::uniformHemiSphere(uv, hitNormal);
	const Scalar probability = 1.0_simd;

	//Vector4 newDirection = lambertianDirection(uv, hitNormal);
	//const Scalar probability = 1.0_simd / Scalar(PI);	// PDF from cosine weighted direction, if uniform then this should be 1.

	Scalar cosPhi = dot3(newDirection, hitNormal);

	Color4f incoming = traceSinglePath(hitOrigin, newDirection, random, depth + 1);

	Color4f direct = sampleAnalyticalLights(
		random,
		hitOrigin,
		hitNormal,
		Light::LmIndirect,
		true
	);

	return emittance + (incoming * BRDF * cosPhi / probability) + direct * hitMaterialColor * cosPhi;
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
			Vector2 uv = Quasirandom::hammersley(i + j, sampleCount, random);
			Vector4 direction = Quasirandom::uniformHemiSphere(uv, normal);
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

	float k = float(unoccluded) / sampleCount;
	float o = std::pow(k, 2.0f);

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
						Vector4 lumelPosition = origin;
						Vector4 traceDirection = -light.direction;
						Vector2 uv = m_shadowSampleOffsets[j];
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
				Scalar lightDistance = lightDirection.normalize();
				if (lightDistance > light.range)
					break;

				Scalar phi = dot3(normal, lightDirection);
				if (phi <= 0.0f)
					break;

				Scalar f = attenuation(lightDistance, light.range);
				if (f <= 0.0f)
					break;

				Scalar shadowAttenuate(1.0f);

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
						Vector4 lumelPosition = origin;
						Vector4 traceDirection = (light.position - origin).xyz0().normalized();
						Vector2 uv = m_shadowSampleOffsets[j];
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

				contribution += light.color * phi * min(f, Scalar(1.0f)) * shadowAttenuate;
			}
			break;

		case Light::LtSpot:
			{
				Vector4 lightToPoint = (origin - light.position).xyz0();
				Scalar lightDistance = lightToPoint.normalize();
				if (lightDistance > light.range)
					break;

				float alpha = clamp< float >(dot3(light.direction, lightToPoint), -1.0f, 1.0f);
				Scalar k0 = Scalar(1.0f - std::acos(alpha) / (light.radius / 2.0f));
				if (k0 <= 0.0f)
					break;

				Scalar k1 = dot3(normal, -lightToPoint);
				if (k1 <= 0.0f)
					break;

				Scalar k2 = attenuation(lightDistance, light.range);
				if (k2 <= 0.0f)
					break;

				Scalar shadowAttenuate(1.0f);

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
						Vector4 lumelPosition = origin;
						Vector4 traceDirection = (light.position - origin).xyz0().normalized();
						Vector2 uv = m_shadowSampleOffsets[j];
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

		uint32_t geomID = RTCHitN_geomID(hits, args->N, i);
		uint32_t primID = RTCHitN_primID(hits, args->N, i);

		const auto& polygons = self->m_models[geomID]->getPolygons();
		const auto& materials = self->m_models[geomID]->getMaterials();

		const auto& hitPolygon = polygons[primID];
		const auto& hitMaterial = materials[hitPolygon.getMaterial()];

		if (hitMaterial.getBlendOperator() != model::Material::BoAlphaTest)
			continue;

		const auto& image = hitMaterial.getDiffuseMap().image;
		if (image)
		{
			const uint32_t slot = 0;
			float texCoord[2] = { 0.0f, 0.0f };

			float u = RTCHitN_u(hits, args->N, i);
			float v = RTCHitN_v(hits, args->N, i);

			RTCGeometry geometry = rtcGetGeometry(self->m_scene, geomID);
			rtcInterpolate0(geometry, primID, u, v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

			if (image->getPixel(
				(int32_t)(wrap(texCoord[0]) * image->getWidth()),
				(int32_t)(wrap(texCoord[1]) * image->getHeight()),
				color
			))
			{
				if (color.getAlpha() <= 0.5f)
					args->valid[i] = 0;
			}
		}
	}
}

    }
}
