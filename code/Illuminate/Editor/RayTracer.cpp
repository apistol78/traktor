#include "Illuminate/Editor/IlluminateConfiguration.h"
#include "Illuminate/Editor/RayTracer.h"
#include "Model/Model.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

const Scalar c_epsilonOffset(0.0001f);

Scalar attenuation(const Scalar& distance)
{
	return clamp(
		Scalar(1.0f) / distance,
		Scalar(0.0f),
		Scalar(1.0f)
	);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.RayTracer", RayTracer, Object)

RayTracer::RayTracer(const IlluminateConfiguration* configuration)
:	m_configuration(configuration)
,	m_maxDistance(1000.0f)
{
}

void RayTracer::addLight(const Light& light)
{
	m_lights.push_back(light);
}

void RayTracer::addModel(const model::Model* model, const Transform& transform)
{
	const auto& polygons = model->getPolygons();

	// Construct simple windins from each model's polygon.
	m_windings.reserve(m_windings.size() + polygons.size());
	for (const auto& polygon : polygons)
	{
		auto& w = m_windings.push_back();
		for (const auto index : polygon.getVertices())
		{
			const auto& vertex = model->getVertex(index);
			const auto& position = model->getPosition(vertex.getPosition());
			w.push(transform * position.xyz1());
		}
	}
}

bool RayTracer::prepare()
{
	m_sah.build(m_windings);
	return true;
}

Color4f RayTracer::traceDirect(const Vector4& origin, const Vector4& normal, float roughness)
{
	return sampleAnalyticalLights(
		origin,
		normal,
		m_configuration->getShadowSampleCount(),
		m_configuration->getPointLightShadowRadius()
	);
}

Color4f RayTracer::traceIndirect(const Vector4& origin, const Vector4& normal, float roughness)
{
	SahTree::QueryResult result;

	Color4f irradiance(0.0f, 0.0f, 0.0f, 0.0f);
	for (uint32_t i = 0; i < m_configuration->getIrradianceSampleCount(); ++i)
	{
		Vector4 direction = lerp(m_random.nextHemi(normal), normal, Scalar(roughness)).normalized();
		if (m_sah.queryClosestIntersection(origin + normal * c_epsilonOffset, direction, m_maxDistance, -1, result, m_sahCache))
		{
			Color4f contribution = sampleAnalyticalLights(
				result.position,
				result.normal,
				1,
				0.0f
			);
			irradiance += contribution * attenuation((result.position - origin).xyz0().length());
		}
	}
	irradiance /= Scalar(m_configuration->getIrradianceSampleCount());

	return irradiance;
}

Scalar RayTracer::traceOcclusion(const Vector4& origin, const Vector4& normal)
{
	const float occlusionDistance = 10.0f;
	
	uint32_t occlusion = 0;
	for (uint32_t i = 0; i < m_configuration->getOcclusionSampleCount(); ++i)
	{
		Vector4 occlusionDirection = m_random.nextHemi(normal);
		if (m_sah.queryAnyIntersection(origin + normal * c_epsilonOffset, occlusionDirection, occlusionDistance, -1, m_sahCache))
			occlusion++;
	}

	return Scalar((float)occlusion / m_configuration->getOcclusionSampleCount());
}

Color4f RayTracer::sampleAnalyticalLights(const Vector4& origin, const Vector4& normal, uint32_t shadowSampleCount, float pointLightShadowRadius)
{
	Color4f contribution(0.0f, 0.0f, 0.0f, 0.0f);
	for (const auto& light : m_lights)
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
						m_sahCache
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

				Scalar f = Scalar(1.0f) - lightDistance / Scalar(light.range);
				if (f <= 0.0f)
					break;

				Scalar shadowAttenuate(1.0f);
				int32_t shadowCount = 0;

				Vector4 u, v;
				orthogonalFrame(lightDirection, u, v);

				for (uint32_t j = 0; j < shadowSampleCount; ++j)
				{
					float a, b;
					do
					{
						a = m_random.nextFloat() * 2.0f - 1.0f;
						b = m_random.nextFloat() * 2.0f - 1.0f;
					}
					while ((a * a) + (b * b) > 1.0f);

					Vector4 shadowDirection = (light.position + u * Scalar(a * pointLightShadowRadius) + v * Scalar(b * pointLightShadowRadius) - origin).xyz0();

					if (m_sah.queryAnyIntersection(origin + normal * c_epsilonOffset, shadowDirection.normalized(), lightDistance - c_epsilonOffset, -1, m_sahCache))
						shadowCount++;
				}
				shadowAttenuate = Scalar(1.0f - float(shadowCount) / m_configuration->getShadowSampleCount());

				contribution += light.color * phi * min(f, Scalar(1.0f)) * shadowAttenuate;
			}
			break;

		case Light::LtProbe:
			{

			}
			break;
		}
	}
	return contribution;
}

	}
}