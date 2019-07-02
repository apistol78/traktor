#include <functional>
#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>
#include "Core/Functor/Functor.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Render/SH/SHEngine.h"
#include "Render/SH/SHFunction.h"
#include "Shape/Editor/Bake/GBuffer.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/Embree/RayTracerEmbree.h"

namespace traktor
{
    namespace shape
    {
		namespace
		{

const Scalar p(1.0f / (2.0f * PI));
const float c_epsilonOffset = 0.01f;
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
    m_maxDistance = 1000.0f;

	m_device = rtcNewDevice(nullptr);
	m_scene = rtcNewScene(m_device);

	m_shEngine = new render::SHEngine(3);
	m_shEngine->generateSamplePoints(20000);

    return true;
}

void RayTracerEmbree::destroy()
{
	m_shEngine = nullptr;
}

void RayTracerEmbree::addLight(const Light& light)
{
    m_lights.push_back(light);
}

void RayTracerEmbree::addModel(const model::Model* model, const Transform& transform)
{
	const auto& polygons = model->getPolygons();

	RTCGeometry mesh = rtcNewGeometry(m_device, RTC_GEOMETRY_TYPE_TRIANGLE);

	float* vertices = (float*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), model->getPositions().size());
	for (const auto& position : model->getPositions())
	{
		Vector4 p = transform * position.xyz1();
		*vertices++ = p.x();
		*vertices++ = p.y();
		*vertices++ = p.z();
	}

	uint32_t* triangles = (uint32_t*)rtcSetNewGeometryBuffer(mesh, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(uint32_t), model->getPolygons().size());
	for (const auto& polygon : model->getPolygons())
	{
		*triangles++ = model->getVertex(polygon.getVertex(2)).getPosition();
		*triangles++ = model->getVertex(polygon.getVertex(1)).getPosition();
		*triangles++ = model->getVertex(polygon.getVertex(0)).getPosition();
	}

	rtcCommitGeometry(mesh);
	rtcAttachGeometry(m_scene, mesh);
	rtcReleaseGeometry(mesh);
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

				Vector4 position = elm.position;
				Vector4 normal = elm.normal;

				Vector4 u, v;
				orthogonalFrame(normal, u, v);

				const Scalar l = elm.delta.length();
				const Scalar hl = l * Scalar(0.3f);
				const Vector4 d[] = { u, -u, v, -v };

				for (int32_t i = 0; i < 4; ++i)
				{
					Vector4 traceOrigin = position + normal * hl;
					Vector4 traceDirection = d[i];

					rh.ray.org_x = traceOrigin.x();
					rh.ray.org_y = traceOrigin.y();
					rh.ray.org_z = traceOrigin.z();

					rh.ray.dir_x = traceDirection.x();
					rh.ray.dir_y = traceDirection.y();
					rh.ray.dir_z = traceDirection.z();

					rh.ray.tnear = 0.0f;
					rh.ray.time = 0.0f;
					rh.ray.tfar = hl;

					rh.ray.mask = 0;
					rh.ray.id = 0;
					rh.ray.flags = 0;

					rh.hit.geomID = RTC_INVALID_GEOMETRY_ID;
					rh.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

					RTCIntersectContext context;
					rtcInitIntersectContext(&context);
					rtcIntersect1(m_scene, &context, &rh);					

					if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
						continue;

					Vector4 hitNormal = Vector4(rh.hit.Ng_x, rh.hit.Ng_y, rh.hit.Ng_z, 0.0f).normalized();

					if (dot3(hitNormal, traceDirection) > 0.0f)
						hitNormal = -hitNormal;

					// Project hit normal onto lightmap normal.
					Scalar k = dot3(hitNormal, normal);
					hitNormal = (hitNormal - normal * k).normalized();

					// Offset position.
					//position = position + /*traceDirection * Scalar(rh.ray.tfar) + */hitNormal * Scalar(hl/* - rh.ray.tfar*/);
					position = position + traceDirection * Scalar(-hl);
				}

				elm.position = position;
			}
		}
	}	
}

Ref< render::SHCoeffs > RayTracerEmbree::traceProbe(const Vector4& position) const
{
	const uint32_t sampleCount = alignUp(m_configuration->getIndirectSampleCount(), 16);
	RandomGeometry random;

	WrappedSHFunction shFunction([&] (const Vector4& unit) -> Vector4 {
		Color4f indirect(0.0f, 0.0f, 0.0f, 0.0f);
		RTCRayHit16 T_MATH_ALIGN16 rh;
		Vector4 direction[16];

		for (uint32_t i = 0; i < sampleCount; i += 16)
		{
			for (uint32_t j = 0; j < 16; ++j)
			{
				direction[j] = random.nextHemi(unit);

				rh.ray.org_x[j] = position.x();
				rh.ray.org_y[j] = position.y();
				rh.ray.org_z[j] = position.z();

				rh.ray.dir_x[j] = direction[j].x();
				rh.ray.dir_y[j] = direction[j].y();
				rh.ray.dir_z[j] = direction[j].z();

				rh.ray.tnear[j] = c_epsilonOffset;
				rh.ray.time[j] = 0.0f;
				rh.ray.tfar[j] = m_maxDistance;

				rh.ray.mask[j] = 0;
				rh.ray.id[j] = 0;
				rh.ray.flags[j] = 0;

				rh.hit.geomID[j] = RTC_INVALID_GEOMETRY_ID;
				rh.hit.instID[0][j] = RTC_INVALID_GEOMETRY_ID;
			}

			RTCIntersectContext context;
			rtcInitIntersectContext(&context);
			rtcIntersect16(c_valid, m_scene, &context, &rh);

			for (uint32_t j = 0; j < 16; ++j)
			{
				if (rh.hit.geomID[j] != RTC_INVALID_GEOMETRY_ID)
				{
					Scalar distance(rh.ray.tfar[j]);
					Scalar f = attenuation(distance);
					if (f <= 0.0f)
						continue;

					Vector4 hitPosition = (position + direction[j] * distance).xyz1();
					Vector4 hitNormal = Vector4(rh.hit.Ng_x[j], rh.hit.Ng_y[j], rh.hit.Ng_z[j], 0.0f).normalized();

					Scalar ct = dot3(unit, direction[j]);
					Color4f brdf = /*m_surfaces[result.index].albedo*/ Color4f(1.0f, 1.0f, 1.0f, 0.0f) / Scalar(PI);
					Color4f incoming = sampleAnalyticalLights(
						random,
						hitPosition,
						hitNormal,
						true
					);
					indirect += brdf * incoming * f * ct / p;
				}
			}
		}

		indirect /= Scalar(sampleCount);
		return indirect;
	});

	Ref< render::SHCoeffs > shCoeffs = new render::SHCoeffs();
	m_shEngine->generateCoefficients(&shFunction, *shCoeffs);

	return shCoeffs;
}

Ref< drawing::Image > RayTracerEmbree::traceDirect(const GBuffer* gbuffer) const
{
	RandomGeometry random;

    int32_t width = gbuffer->getWidth();
    int32_t height = gbuffer->getHeight();

	const int32_t sampleCount = 16;

    Ref< drawing::Image > lightmapDirect = new drawing::Image(drawing::PixelFormat::getRGBAF32(), width, height);
    lightmapDirect->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	for (int32_t y = 0; y < height; ++y)
	{
		for (int32_t x = 0; x < width; ++x)
		{
            const auto& elm = gbuffer->get(x, y);
            if (elm.polygon == model::c_InvalidIndex)
                continue;

			Scalar hl = elm.delta.length() * Scalar(0.3f);

			Vector4 u, v;
			orthogonalFrame(elm.normal, u, v);

			Color4f direct(0.0f, 0.0f, 0.0f, 0.0f);
			for (int32_t i = 0; i < sampleCount; ++i)
			{
				float fu = random.nextFloat() * 2.0f - 1.0f;
				float fv = random.nextFloat() * 2.0f - 1.0f;

				Vector4 position = elm.position + (u * Scalar(fu) + v * Scalar(fv)) * hl;

				direct += sampleAnalyticalLights(
					random,
					position,
					elm.normal,
					false
				);
			}
			direct /= Scalar(sampleCount);

            lightmapDirect->setPixel(x, y, direct.rgb1());
		}
	}

    return lightmapDirect;
}

Ref< drawing::Image > RayTracerEmbree::traceIndirect(const GBuffer* gbuffer) const
{
	RandomGeometry random;

    int32_t width = gbuffer->getWidth();
    int32_t height = gbuffer->getHeight();

    Ref< drawing::Image > lightmapIndirect = new drawing::Image(drawing::PixelFormat::getRGBAF32(), width, height);
    lightmapIndirect->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	const uint32_t sampleCount = alignUp(m_configuration->getIndirectSampleCount(), 16);

    RefArray< Job > jobs;
    for (int32_t ty = 0; ty < height; ty += 16)
    {
        for (int32_t tx = 0; tx < width; tx += 16)
        {
            auto job = JobManager::getInstance().add(makeFunctor([&, tx, ty]() {
				RTCRayHit16 T_MATH_ALIGN16 rh;
				Vector4 direction[16];

                for (int32_t y = ty; y < ty + 16; ++y)
                {
                    for (int32_t x = tx; x < tx + 16; ++x)
                    {
						const auto& elm = gbuffer->get(x, y);
						if (elm.polygon == model::c_InvalidIndex)
							continue;

						Color4f indirect(0.0f, 0.0f, 0.0f, 0.0f);

						for (uint32_t i = 0; i < sampleCount; i += 16)
						{
							for (uint32_t j = 0; j < 16; ++j)
							{
								direction[j] = (elm.normal * Scalar(0.2f) + random.nextHemi(elm.normal)).normalized();

								rh.ray.org_x[j] = elm.position.x();
								rh.ray.org_y[j] = elm.position.y();
								rh.ray.org_z[j] = elm.position.z();

								rh.ray.dir_x[j] = direction[j].x();
								rh.ray.dir_y[j] = direction[j].y();
								rh.ray.dir_z[j] = direction[j].z();

								rh.ray.tnear[j] = c_epsilonOffset;
								rh.ray.time[j] = 0.0f;
								rh.ray.tfar[j] = m_maxDistance;

								rh.ray.mask[j] = 0;
								rh.ray.id[j] = 0;
								rh.ray.flags[j] = 0;

								rh.hit.geomID[j] = RTC_INVALID_GEOMETRY_ID;
								rh.hit.instID[0][j] = RTC_INVALID_GEOMETRY_ID;
							}

							RTCIntersectContext context;
							rtcInitIntersectContext(&context);
							rtcIntersect16(c_valid, m_scene, &context, &rh);

							for (uint32_t j = 0; j < 16; ++j)
							{
								if (rh.hit.geomID[j] != RTC_INVALID_GEOMETRY_ID)
								{
									Scalar distance(rh.ray.tfar[j]);
									Scalar f = attenuation(distance);
									if (f <= 0.0f)
										continue;

									Vector4 hitPosition = (elm.position + direction[j] * distance).xyz1();
									Vector4 hitNormal = Vector4(rh.hit.Ng_x[j], rh.hit.Ng_y[j], rh.hit.Ng_z[j], 0.0f).normalized();

									if (dot3(hitNormal, direction[j]) > 0.0f)
										continue;

									Scalar ct = dot3(elm.normal, direction[j]);
									if (ct < 0.0f)
										ct = Scalar(0.0f);

									Color4f brdf = /*m_surfaces[result.index].albedo*/ Color4f(1.0f, 1.0f, 1.0f, 0.0f) / Scalar(PI);
									Color4f incoming = sampleAnalyticalLights(
										random,
										hitPosition,
										hitNormal,
										true
									);
									indirect += brdf * incoming * f * ct / p;
								}
							}
						}

						indirect /= Scalar(sampleCount);

						lightmapIndirect->setPixel(x, y, indirect.rgb1());
					}
				}
            }));
            if (!job)
               return nullptr;

            jobs.push_back(job);
		}
	}
    while (!jobs.empty())
    {
        jobs.back()->wait();
        jobs.pop_back();
    }

    return lightmapIndirect;
}

Ref< drawing::Image > RayTracerEmbree::traceCamera(const Transform& transform, int32_t width, int32_t height, float fov) const
{
	Ref< drawing::Image > image = new drawing::Image(drawing::PixelFormat::getRGBAF32(), width, height);
	image->clear(Color4f(0.0f, 0.0f, 0.0f, 1.0f));

	RandomGeometry random;
	RTCRayHit T_MATH_ALIGN16 rh;

	const uint32_t sampleCount = alignUp(m_configuration->getIndirectSampleCount(), 16);
	const float ratio = width / (float)height;

	for (int32_t y = 0; y < height; ++y)
	{
		for (int32_t x = 0; x < width; ++x)
		{
			float fx = (2.0f * ((x + 0.5f) / width) - 1.0f) * std::tan(fov / 2.0f) * ratio; 
			float fy = (1.0f - 2.0f * ((y + 0.5f) / height)) * std::tan(fov / 2.0f);

			Vector4 traceOrigin = transform.translation().xyz1();
			Vector4 traceDirection = (transform * Vector4(fx, fy, 1.0f, 0.0f)).normalized();

			rh.ray.org_x = traceOrigin.x();
			rh.ray.org_y = traceOrigin.y();
			rh.ray.org_z = traceOrigin.z();

			rh.ray.dir_x = traceDirection.x();
			rh.ray.dir_y = traceDirection.y();
			rh.ray.dir_z = traceDirection.z();

			rh.ray.tnear = 0.0f;
			rh.ray.time = 0.0f;
			rh.ray.tfar = 1000.0f;

			rh.ray.mask = 0;
			rh.ray.id = 0;
			rh.ray.flags = 0;

			rh.hit.geomID = RTC_INVALID_GEOMETRY_ID;
			rh.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

			RTCIntersectContext context;
			rtcInitIntersectContext(&context);
			rtcIntersect1(m_scene, &context, &rh);				

			if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
				continue;

			Scalar hitDistance(rh.ray.tfar);
			Vector4 hitPosition = (traceOrigin + traceDirection * hitDistance).xyz1();
			Vector4 hitNormal = Vector4(rh.hit.Ng_x, rh.hit.Ng_y, rh.hit.Ng_z, 0.0f).normalized();

			Color4f direct = sampleAnalyticalLights(
				random,
				hitPosition,
				hitNormal,
				false
			);

			Color4f indirect(0.0f, 0.0f, 0.0f, 0.0f);
			for (uint32_t i = 0; i < sampleCount; i += 16)
            {
				Vector4 traceDirection = random.nextHemi(hitNormal);

				rh.ray.org_x = hitPosition.x();
				rh.ray.org_y = hitPosition.y();
				rh.ray.org_z = hitPosition.z();

				rh.ray.dir_x = traceDirection.x();
				rh.ray.dir_y = traceDirection.y();
				rh.ray.dir_z = traceDirection.z();

				rh.ray.tnear = c_epsilonOffset;
				rh.ray.time = 0.0f;
				rh.ray.tfar = m_maxDistance;

				rh.ray.mask = 0;
				rh.ray.id = 0;
				rh.ray.flags = 0;

				rh.hit.geomID = RTC_INVALID_GEOMETRY_ID;
				rh.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

				RTCIntersectContext context;
				rtcInitIntersectContext(&context);
				rtcIntersect1(m_scene, &context, &rh);					

				if (rh.hit.geomID != RTC_INVALID_GEOMETRY_ID)
				{
					Scalar distance(rh.ray.tfar);
					Scalar f = attenuation(distance);
					if (f <= 0.0f)
						continue;

					Vector4 hitPosition2 = (hitPosition + traceDirection * distance).xyz1();
					Vector4 hitNormal2 = Vector4(rh.hit.Ng_x, rh.hit.Ng_y, rh.hit.Ng_z, 0.0f).normalized();

					Scalar ct = dot3(hitNormal2, traceDirection);
					Color4f brdf = /*m_surfaces[result.index].albedo*/ Color4f(1.0f, 1.0f, 1.0f, 0.0f) / Scalar(PI);
					Color4f incoming = sampleAnalyticalLights(
						random,
						hitPosition,
						hitNormal,
						true
					);
					indirect += brdf * incoming * f * ct / p;
				}
            }

            indirect /= Scalar(m_configuration->getIndirectSampleCount());

			image->setPixelUnsafe(x, y, (direct + indirect).rgb1());
		}
	}

	return image;
}

Color4f RayTracerEmbree::sampleAnalyticalLights(
    RandomGeometry& random,
    const Vector4& origin,
    const Vector4& normal,
	bool secondary
 ) const
{
	const uint32_t shadowSampleCount = !secondary ? m_configuration->getShadowSampleCount() : (m_configuration->getShadowSampleCount() > 0 ? 1 : 0);
    const float shadowRadius = !secondary ? m_configuration->getPointLightShadowRadius() : 0.0f;
	RTCRay T_MATH_ALIGN16 r;

	Color4f contribution(0.0f, 0.0f, 0.0f, 0.0f);
	for (const auto& light : m_lights)
	{
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
