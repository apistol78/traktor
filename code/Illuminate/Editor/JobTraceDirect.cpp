/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <ctime>
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Drawing/Image.h"
#include "Illuminate/Editor/CubeProbe.h"
#include "Illuminate/Editor/GBuffer.h"
#include "Illuminate/Editor/JobTraceDirect.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

const int32_t c_jobTileSize = 128;
const Scalar c_traceOffset(0.01f);

		}

JobTraceDirect::JobTraceDirect(
	int32_t tileX,
	int32_t tileY,
	const SahTree& sah,
	const GBuffer& gbuffer,
	const AlignedVector< Light >& lights,
	drawing::Image* outputImageDirect,
	const drawing::Image* imageOcclusion,
	float pointLightRadius,
	int32_t shadowSamples,
	int32_t probeSamples,
	float probeCoeff,
	float probeSpread,
	float probeShadowSpread
)
:	m_tileX(tileX)
,	m_tileY(tileY)
,	m_sah(sah)
,	m_gbuffer(gbuffer)
,	m_lights(lights)
,	m_outputImageDirect(outputImageDirect)
,	m_imageOcclusion(imageOcclusion)
,	m_pointLightRadius(pointLightRadius)
,	m_shadowSamples(shadowSamples)
,	m_probeSamples(probeSamples)
,	m_probeCoeff(probeCoeff)
,	m_probeSpread(probeSpread)
,	m_probeShadowSpread(probeShadowSpread)
{
}

void JobTraceDirect::execute()
{
	RandomGeometry random(std::clock());
	SahTree::QueryCache cache;
	SahTree::QueryResult result;
	Color4f tmp;

	for (int32_t y = m_tileY; y < m_tileY + c_jobTileSize; ++y)
	{
		for (int32_t x = m_tileX; x < m_tileX + c_jobTileSize; ++x)
		{
			const GBuffer::Element& gb = m_gbuffer.get(x, y);

			if (gb.surfaceIndex < 0)
				continue;

			Color4f occlusion(0.0f, 0.0f, 0.0f, 0.0f);
			m_imageOcclusion->getPixel(x, y, occlusion);

			Color4f radiance(0.0f, 0.0f, 0.0f, 0.0f);

			for (AlignedVector< Light >::const_iterator i = m_lights.begin(); i != m_lights.end(); ++i)
			{
				if (i->type == 0)	// Directional
				{
					Vector4 lightDirection = -i->direction;

					// Cosine
					Scalar phi = dot3(lightDirection, gb.normal);
					if (phi <= 0.0f)
						continue;

					// Shadow
					Vector4 u, v;
					orthogonalFrame(lightDirection, u, v);

					int32_t shadowCount = 0;
					for (int32_t j = 0; j < m_shadowSamples; ++j)
					{
						float a, b;
						do
						{
							a = random.nextFloat() * 2.0f - 1.0f;
							b = random.nextFloat() * 2.0f - 1.0f;
						}
						while ((a * a) + (b * b) > 1.0f);

						Vector4 shadowPosition = gb.position + u * Scalar(a * m_pointLightRadius) + v * Scalar(b * m_pointLightRadius);
						Vector4 shadowOrigin = (shadowPosition + gb.normal * c_traceOffset).xyz1();
						if (m_sah.queryAnyIntersection(shadowOrigin, lightDirection, 1e3f, gb.surfaceIndex, cache))
							shadowCount++;
					}
					Scalar shadowAttenuate = Scalar(1.0f - float(shadowCount) / m_shadowSamples);

					radiance += i->sunColor * shadowAttenuate * phi;
				}
				else if (i->type == 1)	// Point
				{
					Vector4 lightDirection = (i->position - gb.position).xyz0();

					// Distance
					Scalar lightDistance = lightDirection.normalize();
					if (lightDistance >= i->range)
						continue;

					Scalar distanceAttenuate = Scalar(1.0f) - lightDistance / i->range;

					// Cosine
					Scalar phi = dot3(lightDirection, gb.normal);
					if (phi <= 0.0f)
						continue;

					// Shadow
					Vector4 u, v;
					orthogonalFrame(lightDirection, u, v);

					Scalar shadowAttenuate(1.0f);
					if (m_shadowSamples > 0)
					{
						int32_t shadowCount = 0;
						for (int32_t j = 0; j < m_shadowSamples; ++j)
						{
							float a, b;
							do
							{
								a = random.nextFloat() * 2.0f - 1.0f;
								b = random.nextFloat() * 2.0f - 1.0f;
							}
							while ((a * a) + (b * b) > 1.0f);

							Vector4 shadowDirection = (i->position + u * Scalar(a * m_pointLightRadius) + v * Scalar(b * m_pointLightRadius) - gb.position).xyz0();

							if (m_sah.queryAnyIntersection(gb.position + gb.normal * c_traceOffset, shadowDirection.normalized(), lightDistance - FUZZY_EPSILON, gb.surfaceIndex, cache))
								shadowCount++;
						}
						shadowAttenuate = Scalar(1.0f - float(shadowCount) / m_shadowSamples);
					}

					radiance += i->sunColor * shadowAttenuate * distanceAttenuate * phi;
				}
				else if (i->type == 2)	// Probe
				{
					radiance += i->probe->sample(gb.normal) * Scalar(m_probeCoeff);
				}
			}

			radiance *= occlusion.getRed();

			m_outputImageDirect->setPixel(x, y, Color4f(Vector4(radiance).xyz1()));
		}
	}
}

	}
}
