/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Shape/Editor/Bake/GBuffer.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/Local/RayTracerLocal.h"

namespace traktor::shape
{
	namespace
	{

const Scalar p(1.0f / (2.0f * PI));
const Scalar c_epsilonOffset(0.1f);

// class WrappedSHFunction : public render::SHFunction
// {
// public:
// 	WrappedSHFunction(const RayTracer& tracer, RayTracer::Context* tracerContext, const Vector4& origin)
// 	:	m_tracer(tracer)
// 	,	m_tracerContext(tracerContext)
// 	,	m_origin(origin)
// 	{
// 	}

// 	virtual Vector4 evaluate(float phi, float theta, const Vector4& unit) const override final
// 	{
// 		return m_tracer.traceIndirect(m_tracerContext, m_origin, unit);
// 	}

// private:
// 	const RayTracer& m_tracer;
// 	Ref< RayTracer::Context > m_tracerContext;
// 	Vector4 m_origin;
// };

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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.RayTracerLocal", 0, RayTracerLocal, IRayTracer)

bool RayTracerLocal::create(const BakeConfiguration* configuration)
{
	m_configuration = configuration;
    m_maxDistance = 100.0f;
    return true;
}

void RayTracerLocal::destroy()
{
}

void RayTracerLocal::addEnvironment(const IProbe* environment)
{
}

void RayTracerLocal::addLight(const Light& light)
{
    m_lights.push_back(light);
}

void RayTracerLocal::addModel(const model::Model* model, const Transform& transform)
{
	const auto& polygons = model->getPolygons();

	// Construct simple windings and surfaces from each model's polygon.
	m_windings.reserve(m_windings.size() + polygons.size());
	m_surfaces.reserve(m_surfaces.size() + polygons.size());
	for (const auto& polygon : polygons)
	{
		auto& w = m_windings.push_back();
		for (const auto index : polygon.getVertices())
		{
			const auto& vertex = model->getVertex(index);
			const auto& position = model->getPosition(vertex.getPosition());
			w.push(transform * position.xyz1());
		}

		auto& s = m_surfaces.push_back();
		const auto& material = model->getMaterial(polygon.getMaterial());
		s.albedo = material.getColor();
	}
}

void RayTracerLocal::commit()
{
	m_sah.build(m_windings);
}

Ref< render::SHCoeffs > RayTracerLocal::traceProbe(const Vector4& position, const Vector4& size) const
{
	// // Raytrace IBL probes.
	// render::SHEngine shEngine(3);
	// shEngine.generateSamplePoints(20000);

	// RefArray< Job > jobs;
	// for (uint32_t i = 0; i < lightEntityDatas.size(); ++i)
	// {
	// 	auto lightEntityData = lightEntityDatas[i];
	// 	T_FATAL_ASSERT(lightEntityData != nullptr);

	// 	auto lightComponentData = lightEntityData->getComponent< world::LightComponentData >();
	// 	T_FATAL_ASSERT(lightComponentData != nullptr);

	// 	if (lightComponentData->getLightType() != world::LtProbe)
	// 		continue;

	// 	log::info << L"Tracing SH probe \"" << lightEntityData->getName() << L"\" (" << i << L"/" << lightEntityDatas.size() << L")..." << Endl;

	// 	auto position = lightEntityData->getTransform().translation().xyz1();

	// 	auto job = JobManager::getInstance().add([&, lightComponentData]() {
	// 		Ref< render::SHCoeffs > shCoeffs = new render::SHCoeffs();

	// 		Ref< RayTracer::Context > context = tracer.createContext();
	// 		WrappedSHFunction shFunction(tracer, context, position);
	// 		shEngine.generateCoefficients(&shFunction, *shCoeffs);

	// 		lightComponentData->setSHCoeffs(shCoeffs);
	// 	});
	// 	if (!job)
	// 		return false;

	// 	jobs.push_back(job);
	// }
	// while (!jobs.empty())
	// {
	// 	jobs.back()->wait();
	// 	jobs.pop_back();
	// }	
	return nullptr;
}

void RayTracerLocal::traceLightmap(const model::Model* model, const GBuffer* gbuffer, drawing::Image* lightmapDiffuse, const int32_t region[4]) const
{
}

Color4f RayTracerLocal::traceRay(const Vector4& position, const Vector4& direction) const
{
	return Color4f(0.0f, 0.0f, 0.0f, 1.0f);
}

/*
Ref< drawing::Image > RayTracerLocal::traceDirect(const GBuffer* gbuffer) const
{
    int32_t width = gbuffer->getWidth();
    int32_t height = gbuffer->getHeight();

	AlignedVector< Light > lights;
	cullLights(gbuffer, lights);

    Ref< drawing::Image > lightmapDirect = new drawing::Image(drawing::PixelFormat::getRGBAF32(), width, height);
    lightmapDirect->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

    RefArray< Job > jobs;
    for (int32_t ty = 0; ty < height; ty += 16)
    {
        for (int32_t tx = 0; tx < width; tx += 16)
        {
            auto job = JobManager::getInstance().add([&, tx, ty]() {
                SahTree::QueryCache sahCache;
                RandomGeometry random;

                for (int32_t y = ty; y < ty + 16; ++y)
                {
                    for (int32_t x = tx; x < tx + 16; ++x)
                    {
                        const auto& elm = gbuffer->get(x, y);
                        if (elm.polygon == model::c_InvalidIndex)
                            continue;

                        Color4f direct = sampleAnalyticalLights(
                            random,
                            sahCache,
							lights,
                            elm.position,
                            elm.normal,
                            m_configuration->getShadowSampleCount(),
                            m_configuration->getPointLightShadowRadius()
                        );

                        lightmapDirect->setPixel(x, y, direct.rgb1());
                    }
                }
            });
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

    return lightmapDirect;
}

Ref< drawing::Image > RayTracerLocal::traceIndirect(const GBuffer* gbuffer) const
{
    int32_t width = gbuffer->getWidth();
    int32_t height = gbuffer->getHeight();

	AlignedVector< Light > lights;
	cullLights(gbuffer, lights);

    Ref< drawing::Image > lightmapIndirect = new drawing::Image(drawing::PixelFormat::getRGBAF32(), width, height);
    lightmapIndirect->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

    RefArray< Job > jobs;
    for (int32_t ty = 0; ty < height; ty += 16)
    {
        for (int32_t tx = 0; tx < width; tx += 16)
        {
            auto job = JobManager::getInstance().add([&, tx, ty]() {
                SahTree::QueryResult result;
                SahTree::QueryCache sahCache;
                RandomGeometry random;

                for (int32_t y = ty; y < ty + 16; ++y)
                {
                    for (int32_t x = tx; x < tx + 16; ++x)
                    {
                        const auto& elm = gbuffer->get(x, y);
                        if (elm.polygon == model::c_InvalidIndex)
                            continue;

                        Color4f indirect(0.0f, 0.0f, 0.0f, 0.0f);
                        for (uint32_t i = 0; i < m_configuration->getIndirectSampleCount(); ++i)
                        {
                            Vector4 direction = random.nextHemi(elm.normal);
                            if (m_sah.queryClosestIntersection(elm.position + elm.normal * c_epsilonOffset, direction, m_maxDistance, -1, result, sahCache))
                            {
								Scalar f = attenuation(result.distance);
								if (f <= 0.0f)
									continue;

                                Scalar ct = dot3(elm.normal, direction);
                                Color4f brdf = m_surfaces[result.index].albedo / Scalar(PI);
                                Color4f incoming = sampleAnalyticalLights(
                                    random,
                                    sahCache,
									lights,
                                    result.position,
                                    result.normal,
                                    1,
                                    0.0f
                                );
                                indirect += brdf * incoming * f * ct / p;
                            }
                        }
                        indirect /= Scalar(m_configuration->getIndirectSampleCount());

                        lightmapIndirect->setPixel(x, y, indirect.rgb1());
                    }
                }
            });
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
*/
void RayTracerLocal::cullLights(const GBuffer* gbuffer, AlignedVector< Light >& outLights) const
{
	for (auto light : m_lights)
	{
		switch (light.type)
		{
		case Light::LtDirectional:
			{
				outLights.push_back(light);
			}
			break;

		case Light::LtPoint:
			{
				if (gbuffer->getBoundingBox().queryIntersectionSphere(light.position, light.range))
					outLights.push_back(light);
			}
			break;

		case Light::LtSpot:
			{
				if (gbuffer->getBoundingBox().queryIntersectionSphere(light.position, light.range))
					outLights.push_back(light);
			}
			break;
		}
	}
}

Color4f RayTracerLocal::sampleAnalyticalLights(
    RandomGeometry& random,
    SahTree::QueryCache& sahCache,
    const AlignedVector< Light >& lights,
    const Vector4& origin,
    const Vector4& normal,
    uint32_t shadowSampleCount,
    float pointLightShadowRadius
 ) const
{
	Color4f contribution(0.0f, 0.0f, 0.0f, 0.0f);
	for (const auto& light : lights)
	{
		switch (light.type)
		{
		case Light::LtDirectional:
			{
				Scalar phi = dot3(normal, -light.direction);
				if (phi > 0.0f)
				{
					if (!m_sah.queryAnyIntersection(
						origin + normal * c_epsilonOffset,
						-light.direction,
						m_maxDistance,
						-1,
						sahCache
					))
					{
						contribution += light.color * phi;
					}
				}
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
						float a = 0.0f, b = 0.0f;
						if (shadowSampleCount > 1)
						{
							do
							{
								a = random.nextFloat() * 2.0f - 1.0f;
								b = random.nextFloat() * 2.0f - 1.0f;
							}
							while ((a * a) + (b * b) > 1.0f);
						}

						Vector4 shadowDirection = (light.position + u * Scalar(a * pointLightShadowRadius) + v * Scalar(b * pointLightShadowRadius) - origin).xyz0();

						if (m_sah.queryAnyIntersection(origin + normal * c_epsilonOffset, shadowDirection.normalized(), lightDistance - c_epsilonOffset, -1, sahCache))
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
						float a = 0.0f, b = 0.0f;
						if (shadowSampleCount > 1)
						{
							do
							{
								a = random.nextFloat() * 2.0f - 1.0f;
								b = random.nextFloat() * 2.0f - 1.0f;
							}
							while ((a * a) + (b * b) > 1.0f);
						}

						Vector4 shadowDirection = (light.position + u * Scalar(a * pointLightShadowRadius) + v * Scalar(b * pointLightShadowRadius) - origin).xyz0();

						if (m_sah.queryAnyIntersection(origin + normal * c_epsilonOffset, shadowDirection.normalized(), lightDistance - c_epsilonOffset, -1, sahCache))
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
