#include <functional>
#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Math/Float.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quasirandom.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/Variance.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Model/Operations/MergeModel.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/GBuffer.h"
#include "Shape/Editor/Bake/IProbe.h"
#include "Shape/Editor/Bake/Embree/RayTracerEmbree.h"

namespace traktor
{
    namespace shape
    {
		namespace
		{

const Scalar p(1.0f / (2.0f * PI));
const float c_epsilonOffset = 0.001f;
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

void constructRay(const Vector4& position, const Vector4& direction, float far, RTCRayHit& outRayHit)
{
	position.storeAligned(&outRayHit.ray.org_x);
	direction.storeAligned(&outRayHit.ray.dir_x);
	outRayHit.ray.tnear = 0.001f;
	outRayHit.ray.time = 0.0f;
	outRayHit.ray.tfar = far;
	outRayHit.ray.mask = 0;
	outRayHit.ray.id = 0;
	outRayHit.ray.flags = 0;
	outRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
	outRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
}

float wrap(float n)
{
	return n - std::floor(n);
}

const Vector4 c_luminance(2.126f, 7.152f, 0.722f, 0.0f);

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

	// Create SH sampling engine.
	m_shEngine = new render::SHEngine(3);
	m_shEngine->generateSamplePoints(
		configuration->getIrradianceSampleCount()
	);

    return true;
}

void RayTracerEmbree::destroy()
{
	m_shEngine = nullptr;
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

	rtcCommitGeometry(mesh);
	rtcAttachGeometry(m_scene, mesh);
	rtcReleaseGeometry(mesh);

	m_models.push_back(model);
}

void RayTracerEmbree::commit()
{
	rtcCommitScene(m_scene);
}

void RayTracerEmbree::preprocess(GBuffer* gbuffer) const
{
	RTCRayHit T_MATH_ALIGN16 rh;

    int32_t width = gbuffer->getWidth();
    int32_t height = gbuffer->getHeight();

	// Offset gbuffer positions to reduce shadowing issues.
	if (m_configuration->getEnableShadowFix())
	{
		for (int32_t y = 0; y < height; ++y)
		{
			for (int32_t x = 0; x < width; ++x)
			{
				auto& elm = gbuffer->get(x, y);
				if (elm.polygon == model::c_InvalidIndex)
					continue;

				const Scalar l = elm.delta;
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

					Vector4 hitNormal = Vector4(rh.hit.Ng_x, rh.hit.Ng_y, rh.hit.Ng_z, 0.0f).normalized();

					if (dot3(hitNormal, traceDirection) < 0.0f)
						continue;

					// Offset position.
					position += traceDirection * Scalar(rh.ray.tfar - 0.001f) + hitNormal * Scalar(0.02f);
				}

				elm.position = position;
			}
		}
	}	
}

Ref< render::SHCoeffs > RayTracerEmbree::traceProbe(const Vector4& position) const
{
	const uint32_t sampleCount = alignUp(m_configuration->getSampleCount(), 16);
	RandomGeometry random;

	// Emperically determined attenuation to better match global lighting.
	const Scalar c_attenuation = 2.0_simd;

	WrappedSHFunction shFunction([&] (const Vector4& unit) -> Vector4 {
		Color4f color(0.0f, 0.0f, 0.0f, 0.0f);
		for (uint32_t i = 0; i < sampleCount; ++i)
		{
			Vector2 uv = Quasirandom::hammersley(i, sampleCount, random);
			Vector4 direction = Quasirandom::uniformHemiSphere(uv, unit);

			// Trace direct analytical illumination, only used for lights
			// of which direct lighting is baked.
			Color4f direct = sampleAnalyticalLights(
				random,
				position,
				direction,
				Light::LmDirect,
				true
			);

			// Incoming indirect light.
			Color4f incoming = tracePath(
				position,
				direction,
				random,
				0
			);

			color += incoming + direct;
		}
		color /= Scalar(sampleCount);
		color *= c_attenuation;
		return color;
	});

	Ref< render::SHCoeffs > shCoeffs = new render::SHCoeffs();
	m_shEngine->generateCoefficients(&shFunction, *shCoeffs);
	return shCoeffs;
}

void RayTracerEmbree::traceLightmap(const model::Model* model, const GBuffer* gbuffer, drawing::Image* lightmap, const int32_t region[4]) const
{
	RandomGeometry random;

	const int32_t sampleCount = m_configuration->getSampleCount();

	const auto& polygons = model->getPolygons();
	const auto& materials = model->getMaterials();

	for (int32_t y = region[1]; y < region[3]; ++y)
	{
		for (int32_t x = region[0]; x < region[2]; ++x)
		{
			const auto& elm = gbuffer->get(x, y);
			if (elm.polygon == model::c_InvalidIndex)
				continue;

			const auto& originPolygon = polygons[elm.polygon];
			const auto& originMaterial = materials[originPolygon.getMaterial()];

			Color4f emittance = originMaterial.getColor() * Scalar(originMaterial.getEmissive());
			Scalar metalness = Scalar(originMaterial.getMetalness());

			// Trace direct analytical illumination.
			Color4f direct = sampleAnalyticalLights(
				random,
				elm.position,
				elm.normal,
				Light::LmDirect,
				false
			);

			// Trace IBL and indirect illumination.
			Color4f incoming(0.0f, 0.0f, 0.0f, 0.0f);
			if (sampleCount > 0)
			{
				for (int32_t i = 0; i < sampleCount; ++i)
				{
					Vector2 uv = Quasirandom::hammersley(i, sampleCount, random);
					Vector4 direction = Quasirandom::uniformHemiSphere(uv, elm.normal);		
			
					incoming += tracePath(
						elm.position,
						direction,
						random,
						0
					);
				}
				incoming /= Scalar(sampleCount);
			}

			lightmap->setPixel(x, y, ((emittance + direct + incoming) * (1.0_simd - metalness)).rgb1());
		}
	}	
}

Color4f RayTracerEmbree::tracePath(
	const Vector4& origin,
	const Vector4& direction,
	RandomGeometry& random,
	int32_t depth
) const
{
	if (depth >= 2)
	{
		// Nothing hit, sample sky if available else it's all black.
		if (m_environment)
			return m_environment->sample(direction);
		else
			return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	}

	RTCRayHit T_MATH_ALIGN16 rh;
	constructRay(origin, direction, 100.0f, rh);

	RTCIntersectContext context;
	rtcInitIntersectContext(&context);
	rtcIntersect1(m_scene, &context, &rh);

	if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
	{
		// Nothing hit, sample sky if available else it's all black.
		if (m_environment)
			return m_environment->sample(direction);
		else
			return Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	}

	const auto& polygons = m_models[rh.hit.geomID]->getPolygons();
	const auto& materials = m_models[rh.hit.geomID]->getMaterials();

	const auto& hitPolygon = polygons[rh.hit.primID];
	const auto& hitMaterial = materials[hitPolygon.getMaterial()];

	Color4f hitMaterialColor = hitMaterial.getColor();
	if (hitMaterial.getDiffuseMap().image)
	{
		const uint32_t slot = 0;
		float texCoord[2] = { 0.0f, 0.0f };

		RTCGeometry geometry = rtcGetGeometry(m_scene, rh.hit.geomID);
		rtcInterpolate0(geometry, rh.hit.primID, rh.hit.u, rh.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, slot, texCoord, 2);

		auto image = hitMaterial.getDiffuseMap().image;
		image->getPixel(
			(int32_t)(wrap(texCoord[0]) * image->getWidth()),
			(int32_t)(wrap(texCoord[1]) * image->getHeight()),
			hitMaterialColor
		);
	}

	Color4f emittance = hitMaterialColor * Scalar(hitMaterial.getEmissive());

	Vector4 hitNormal = Vector4::loadUnaligned(&rh.hit.Ng_x).xyz0().normalized();
	Vector4 newOrigin = (origin + direction * Scalar(rh.ray.tfar)).xyz1();

	Vector2 rnd(random.nextFloat(), random.nextFloat());
	Vector4 newDirection = lambertianDirection(rnd, hitNormal);

	Color4f direct = sampleAnalyticalLights(
		random,
		newOrigin,
		hitNormal,
		Light::LmIndirect,
		true
	);
	Color4f incoming = tracePath(newOrigin, newDirection, random, depth + 1);
	Color4f reflectance = hitMaterialColor;

	return emittance + (direct + incoming) * reflectance;
}

Color4f RayTracerEmbree::sampleAnalyticalLights(
    RandomGeometry& random,
    const Vector4& origin,
    const Vector4& normal,
	uint8_t mask,
	bool bounce
 ) const
{
	const uint32_t shadowSampleCount = !bounce ? m_configuration->getShadowSampleCount() : (m_configuration->getShadowSampleCount() > 0 ? 1 : 0);
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

					int32_t shadowCount = 0;
					for (uint32_t j = 0; j < shadowSampleCount; ++j)
					{
						Vector4 lumelPosition = origin;
						Vector4 traceDirection = -light.direction;

						if (shadowSampleCount > 1)
						{
							float a = 0.0f, b = 0.0f;
							do
							{
								a = random.nextFloat() * 2.0f - 1.0f;
								b = random.nextFloat() * 2.0f - 1.0f;
							}
							while ((a * a) + (b * b) > 1.0f);
							lumelPosition += u * Scalar(a * shadowRadius) + v * Scalar(b * shadowRadius);
						}

						lumelPosition.storeAligned(&r.org_x); 
						traceDirection.storeAligned(&r.dir_x);

						r.tnear = c_epsilonOffset;
						r.time = 0.0f;
						r.tfar = m_maxDistance;

						r.mask = 0;
						r.id = 0;
						r.flags = 0;
		
						RTCIntersectContext context;
						rtcInitIntersectContext(&context);

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

					int32_t shadowCount = 0;
					for (uint32_t j = 0; j < shadowSampleCount; ++j)
					{
						Vector4 lumelPosition = origin;
						Vector4 traceDirection = (light.position - origin).xyz0().normalized();

						if (shadowSampleCount > 1)
						{
							float a = 0.0f, b = 0.0f;
							do
							{
								a = random.nextFloat() * 2.0f - 1.0f;
								b = random.nextFloat() * 2.0f - 1.0f;
							}
							while ((a * a) + (b * b) > 1.0f);
							traceDirection = (light.position + u * Scalar(a * shadowRadius) + v * Scalar(b * shadowRadius) - origin).xyz0().normalized();
						}

						lumelPosition.storeAligned(&r.org_x); 
						traceDirection.storeAligned(&r.dir_x);

						r.tnear = c_epsilonOffset;
						r.time = 0.0f;
						r.tfar = lightDistance - c_epsilonOffset * 2;

						r.mask = 0;
						r.id = 0;
						r.flags = 0;
		
						RTCIntersectContext context;
						rtcInitIntersectContext(&context);

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

					int32_t shadowCount = 0;
					for (uint32_t j = 0; j < shadowSampleCount; ++j)
					{
						Vector4 lumelPosition = origin;
						Vector4 traceDirection = (light.position - origin).xyz0().normalized();

						if (shadowSampleCount > 1)
						{
							float a = 0.0f, b = 0.0f;
							do
							{
								a = random.nextFloat() * 2.0f - 1.0f;
								b = random.nextFloat() * 2.0f - 1.0f;
							}
							while ((a * a) + (b * b) > 1.0f);
							traceDirection = (light.position + u * Scalar(a * shadowRadius) + v * Scalar(b * shadowRadius) - origin).xyz0().normalized();
						}

						lumelPosition.storeAligned(&r.org_x); 
						traceDirection.storeAligned(&r.dir_x);

						r.tnear = c_epsilonOffset;
						r.time = 0.0f;
						r.tfar = lightDistance - c_epsilonOffset * 2;

						r.mask = 0;
						r.id = 0;
						r.flags = 0;
		
						RTCIntersectContext context;
						rtcInitIntersectContext(&context);

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

    }
}
