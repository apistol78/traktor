#include "Core/Log/Log.h"
#include "Illuminate/Editor/IlluminateConfiguration.h"
#include "Illuminate/Editor/RayTracer.h"
#include "Model/Model.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

const Scalar c_epsilonOffset(0.1f);

Scalar attenuation(const Scalar& distance, const Scalar& range)
{
	Scalar k0 = clamp(Scalar(1.0f) / (distance * distance), Scalar(0.0f), Scalar(1.0f));
	Scalar k1 = clamp(Scalar(1.0f) - (distance / range), Scalar(0.0f), Scalar(1.0f));
	return k0 * k1;
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
		s.albedo = Color4f::fromColor4ub(material.getColor());
	}
}

bool RayTracer::prepare()
{
	m_sah.build(m_windings);
	return true;
}

Ref< RayTracer::Context > RayTracer::createContext()
{
	return new RayTracer::Context();
}

bool RayTracer::trace(Context* context, const Vector4& origin, const Vector4& direction, const Scalar& maxDistance, Result& outResult) const
{
	SahTree::QueryResult qr;
	if (m_sah.queryClosestIntersection(origin, direction, maxDistance, -1, qr, context->sahCache))
	{
		outResult.distance = qr.distance;
		outResult.position = qr.position;
		outResult.normal = qr.normal;
		return true;
	}
	else
		return false;
}

Color4f RayTracer::traceDirect(Context* context, const Vector4& origin, const Vector4& normal) const
{
	return sampleAnalyticalLights(
		context,
		origin,
		normal,
		m_configuration->getShadowSampleCount(),
		m_configuration->getPointLightShadowRadius()
	);
}

Color4f RayTracer::traceIndirect(Context* context, const Vector4& origin, const Vector4& normal) const
{
	Color4f irradiance(0.0f, 0.0f, 0.0f, 0.0f);
	SahTree::QueryResult result;

	const Scalar p(1.0f / (2.0f * PI));
	for (uint32_t i = 0; i < m_configuration->getIndirectSampleCount(); ++i)
	{
		Vector4 direction = context->random.nextHemi(normal);
		if (m_sah.queryClosestIntersection(origin + normal * c_epsilonOffset, direction, m_maxDistance, -1, result, context->sahCache))
		{
			Scalar ct = dot3(normal, direction);
			Color4f brdf = m_surfaces[result.index].albedo / Scalar(PI);
			Color4f incoming = sampleAnalyticalLights(
				context,
				result.position,
				result.normal,
				1,
				0.0f
			);
			irradiance += brdf * incoming * ct / p;
		}
	}

	irradiance /= Scalar(m_configuration->getIndirectSampleCount());
	return irradiance;
}

Color4f RayTracer::sampleAnalyticalLights(Context* context, const Vector4& origin, const Vector4& normal, uint32_t shadowSampleCount, float pointLightShadowRadius) const
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
						context->sahCache
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
						float a, b;
						do
						{
							a = context->random.nextFloat() * 2.0f - 1.0f;
							b = context->random.nextFloat() * 2.0f - 1.0f;
						}
						while ((a * a) + (b * b) > 1.0f);

						Vector4 shadowDirection = (light.position + u * Scalar(a * pointLightShadowRadius) + v * Scalar(b * pointLightShadowRadius) - origin).xyz0();

						if (m_sah.queryAnyIntersection(origin + normal * c_epsilonOffset, shadowDirection.normalized(), lightDistance - c_epsilonOffset, -1, context->sahCache))
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
				if (lightDistance <= 0.0f)
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
						float a, b;
						do
						{
							a = context->random.nextFloat() * 2.0f - 1.0f;
							b = context->random.nextFloat() * 2.0f - 1.0f;
						}
						while ((a * a) + (b * b) > 1.0f);

						Vector4 shadowDirection = (light.position + u * Scalar(a * pointLightShadowRadius) + v * Scalar(b * pointLightShadowRadius) - origin).xyz0();

						if (m_sah.queryAnyIntersection(origin + normal * c_epsilonOffset, shadowDirection.normalized(), lightDistance - c_epsilonOffset, -1, context->sahCache))
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