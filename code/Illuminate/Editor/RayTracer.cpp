#include "Illuminate/Editor/RayTracer.h"
#include "Model/Model.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

const Scalar c_epsilonOffset(0.0001f);

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.RayTracer", RayTracer, Object)

RayTracer::RayTracer()
:	m_irradianceSampleCount(64)
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
	return sampleAnalyticalLights(origin, normal);
}

Color4f RayTracer::traceIndirect(const Vector4& origin, const Vector4& normal, float roughness)
{
	SahTree::QueryResult result;

	Color4f irradiance(0.0f, 0.0f, 0.0f, 0.0f);
	for (uint32_t i = 0; i < m_irradianceSampleCount; ++i)
	{
		Vector4 direction = lerp(m_random.nextHemi(normal), normal, Scalar(roughness)).normalized();
		if (m_sah.queryClosestIntersection(origin + normal * c_epsilonOffset, direction, m_maxDistance, -1, result, m_sahCache))
		{
			irradiance += sampleAnalyticalLights(result.position, result.normal) * dot3(normal, direction);
		}
	}
	irradiance /= Scalar(m_irradianceSampleCount);

	return irradiance;
}

Color4f RayTracer::sampleAnalyticalLights(const Vector4& origin, const Vector4& normal)
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
				Scalar lightLength = lightDirection.normalize();
				Scalar phi = dot3(normal, lightDirection);
				Scalar f = Scalar(1.0f) - lightLength / Scalar(light.range);
				if (phi > 0.0f && f > 0.0f)
				{
					if (!m_sah.queryAnyIntersection(
						origin + normal * c_epsilonOffset,
						lightDirection,
						lightLength - c_epsilonOffset * Scalar(2.0f),
						-1,
						m_sahCache
					))
					{
						contribution += light.color * phi * min(f, Scalar(1.0f));
					}
				}
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