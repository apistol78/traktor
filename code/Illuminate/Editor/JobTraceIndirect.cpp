#include <ctime>
#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Drawing/Image.h"
#include "Illuminate/Editor/GBuffer.h"
#include "Illuminate/Editor/JobTraceIndirect.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

const int32_t c_jobTileWidth = 128;
const int32_t c_jobTileHeight = 128;
const Scalar c_maxIndirectDistance(200.0f);
const Scalar c_traceOffset(0.01f);

		}

JobTraceIndirect::JobTraceIndirect(
	int32_t tileX,
	int32_t tileY,
	const SahTree& sah,
	const GBuffer& gbuffer,
	const AlignedVector< Surface >& surfaces,
	const drawing::Image* imageIrradiance,
	drawing::Image* outputImageIndirect,
	int32_t indirectTraceSamples
)
:	m_tileX(tileX)
,	m_tileY(tileY)
,	m_sah(sah)
,	m_gbuffer(gbuffer)
,	m_surfaces(surfaces)
,	m_imageIrradiance(imageIrradiance)
,	m_outputImageIndirect(outputImageIndirect)
,	m_indirectTraceSamples(indirectTraceSamples)
{
}

void JobTraceIndirect::execute()
{
	RandomGeometry random(std::clock());
	SahTree::QueryCache cache;
	SahTree::QueryResult result;
	Color4f irradiance;

	for (int32_t y = m_tileY; y < m_tileY + c_jobTileHeight; ++y)
	{
		for (int32_t x = m_tileX; x < m_tileX + c_jobTileWidth; ++x)
		{
			const GBuffer::Element& gb = m_gbuffer.get(x, y);

			if (gb.surfaceIndex < 0)
				continue;

			Color4f radiance(0.0f, 0.0f, 0.0f, 0.0f);

			for (int32_t i = 0; i < m_indirectTraceSamples; )
			{
				Vector4 rayDirection = random.nextUnit();
				if (dot3(rayDirection, gb.normal) <= 0.2f)
					continue;

				if (m_sah.queryClosestIntersection(gb.position + gb.normal * c_traceOffset, rayDirection, gb.surfaceIndex, result, cache))
				{
					Scalar phi = -dot3(rayDirection, result.normal);
					if (phi <= 0.0f)
					{
						++i;
						continue;
					}

					const Vector4& P = result.position;

					Scalar distance = (P - gb.position).xyz0().length();
					if (distance >= c_maxIndirectDistance)
					{
						++i;
						continue;
					}

					const Surface& hitSurface = m_surfaces[result.index];

					const Vector4& A = hitSurface.points[0];
					const Vector4& B = hitSurface.points[1];
					const Vector4& C = hitSurface.points[2];

					const Vector2& texCoord0 = hitSurface.texCoords[0];
					const Vector2& texCoord1 = hitSurface.texCoords[1];
					const Vector2& texCoord2 = hitSurface.texCoords[2];

					Vector4 v0 = C - A;
					Vector4 v1 = B - A;
					Vector4 v2 = P - A;

					Scalar d00 = dot3(v0, v0);
					Scalar d01 = dot3(v0, v1);
					Scalar d02 = dot3(v0, v2);
					Scalar d11 = dot3(v1, v1);
					Scalar d12 = dot3(v1, v2);

					Scalar invDenom = Scalar(1.0f) / (d00 * d11 - d01 * d01);
					Scalar u = (d11 * d02 - d01 * d12) * invDenom;
					Scalar v = (d00 * d12 - d01 * d02) * invDenom;

					Vector2 texCoord = texCoord0 + (texCoord2 - texCoord0) * u + (texCoord1 - texCoord0) * v;

					int32_t lx = int32_t(texCoord.x * m_imageIrradiance->getWidth());
					int32_t ly = int32_t(texCoord.y * m_imageIrradiance->getHeight());
					m_imageIrradiance->getPixel(lx, ly, irradiance);

					Scalar distanceAttenuate = Scalar(1.0f) - distance / c_maxIndirectDistance;
					radiance += (hitSurface.color * irradiance) * phi * distanceAttenuate;
				}

				++i;
			}

			radiance /= Scalar(m_indirectTraceSamples);
			m_outputImageIndirect->setPixel(x, y, Color4f(Vector4(radiance).xyz1()));
		}
	}
}

	}
}
