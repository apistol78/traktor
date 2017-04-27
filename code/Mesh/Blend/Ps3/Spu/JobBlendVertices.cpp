/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>
#include "Mesh/Blend/Ps3/JobBlendVertices.h"

using namespace traktor;

void cellSpursJobQueueMain(CellSpursJobContext2* context, CellSpursJob256* job256)
{
	mesh::JobBlendVertices* job = (mesh::JobBlendVertices*)job256;

	static uint8_t inputVertexData[8 * 1024];
	static uint8_t outputVertexData[8 * 1024];

	uint32_t vertexOffset = 0;
	for (uint32_t i = 0; i < job->data.vertexCount; i += 128)
	{
		uint32_t vertexCount = job->data.vertexCount - i;
		if (vertexCount > 128)
			vertexCount = 128;

		cellDmaGet(
			inputVertexData,
			job->data.sourceVertices + vertexOffset
			vertexCount * job->data.vertexSize,
			context->dmaTag,
			0,
			0
		);
		cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);
	}

	uint32_t vertexSize = render::getVertexSize(m_vertexElements);
	uint32_t vertexOffset = 0;

	uint8_t* destVerticesTop = m_destinationVertexPtr + vertexOffset;
	const uint8_t* baseVerticesTop = m_sourceVertexPtrs[0] + vertexOffset;

	std::memcpy(destVerticesTop, baseVerticesTop, (m_end - m_start) * vertexSize);

	for (uint32_t i = 0; i < m_blendWeights.size(); ++i)
	{
		float weight = m_blendWeights[i];

		if (std::abs(weight) <= FUZZY_EPSILON)
			continue;

		uint8_t* destVertices = destVerticesTop;
		const uint8_t* targetVertices = m_sourceVertexPtrs[i + 1] + vertexOffset;

		for (uint32_t j = m_start; j < m_end; ++j)
		{
			for (std::vector< render::VertexElement >::const_iterator k = m_vertexElements.begin(); k != m_vertexElements.end(); ++k)
			{
				uint8_t* destVertexElement = destVertices + k->getOffset();
				const uint8_t* targetVertexElement = targetVertices + k->getOffset();

				switch (k->getDataType())
				{
				case render::DtFloat1:
					reinterpret_cast< float* >(destVertexElement)[0] += reinterpret_cast< const float* >(targetVertexElement)[0] * weight;
					break;

				case render::DtFloat2:
					reinterpret_cast< float* >(destVertexElement)[0] += reinterpret_cast< const float* >(targetVertexElement)[0] * weight;
					reinterpret_cast< float* >(destVertexElement)[1] += reinterpret_cast< const float* >(targetVertexElement)[1] * weight;
					break;

				case render::DtFloat3:
					reinterpret_cast< float* >(destVertexElement)[0] += reinterpret_cast< const float* >(targetVertexElement)[0] * weight;
					reinterpret_cast< float* >(destVertexElement)[1] += reinterpret_cast< const float* >(targetVertexElement)[1] * weight;
					reinterpret_cast< float* >(destVertexElement)[2] += reinterpret_cast< const float* >(targetVertexElement)[2] * weight;
					break;

				case render::DtFloat4:
					reinterpret_cast< float* >(destVertexElement)[0] += reinterpret_cast< const float* >(targetVertexElement)[0] * weight;
					reinterpret_cast< float* >(destVertexElement)[1] += reinterpret_cast< const float* >(targetVertexElement)[1] * weight;
					reinterpret_cast< float* >(destVertexElement)[2] += reinterpret_cast< const float* >(targetVertexElement)[2] * weight;
					reinterpret_cast< float* >(destVertexElement)[3] += reinterpret_cast< const float* >(targetVertexElement)[3] * weight;
					break;
				}
			}
			destVertices += vertexSize;
			targetVertices += vertexSize;
		}
	}

}
