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
#include "Illuminate/Editor/GBuffer.h"
#include "Illuminate/Editor/JobTraceOcclusion.h"

namespace traktor
{
	namespace illuminate
	{
		namespace
		{

const int32_t c_jobTileSize = 128;
const Scalar c_traceOffset(0.01f);

		}

JobTraceOcclusion::JobTraceOcclusion(
	int32_t tileX,
	int32_t tileY,
	const SahTree& sah,
	const GBuffer& gbuffer,
	drawing::Image* outputImageOcclusion,
	int32_t occlusionSamples
)
:	m_tileX(tileX)
,	m_tileY(tileY)
,	m_sah(sah)
,	m_gbuffer(gbuffer)
,	m_outputImageOcclusion(outputImageOcclusion)
,	m_occlusionSamples(occlusionSamples)
{
}

void JobTraceOcclusion::execute()
{
	RandomGeometry random(std::clock());
	SahTree::QueryCache cache;
	SahTree::QueryResult result;

	const Scalar traceDistance(4.0f);

	for (int32_t y = m_tileY; y < m_tileY + c_jobTileSize; ++y)
	{
		for (int32_t x = m_tileX; x < m_tileX + c_jobTileSize; ++x)
		{
			const GBuffer::Element& gb = m_gbuffer.get(x, y);

			if (gb.surfaceIndex < 0)
				continue;

			int32_t occludedCount = 0;
			for (int32_t j = 0; j < m_occlusionSamples; ++j)
			{
				Vector4 direction = random.nextHemi(gb.normal);
				if (m_sah.queryAnyIntersection(gb.position + gb.normal * c_traceOffset, direction, traceDistance, gb.surfaceIndex, cache))
					++occludedCount;
			}
			float o = 1.0f - float(occludedCount) / m_occlusionSamples;

			m_outputImageOcclusion->setPixel(x, y, Color4f(o, o, o, o));
		}
	}
}

	}
}
