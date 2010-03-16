#include <cmath>
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>
#include "Spray/Point.h"
#include "Spray/Ps3/Spu/JobModifierUpdate.h"

using namespace traktor;

void cellSpursJobQueueMain(CellSpursJobContext2* context, CellSpursJob256* job256)
{
	spray::JobModifierUpdate* job = (spray::JobModifierUpdate*)job256;

	static spray::Point points[128];

	Scalar deltaTime(job->deltaTime);
	Vector4 gravity(job->gravity.gravity);

	for (uint32_t i = 0; i < job->pointsCount; i += sizeof_array(points))
	{
		uint32_t pointsCount = job->pointsCount - i;
		if (pointsCount > sizeof_array(points))
			pointsCount = sizeof_array(points);

		cellDmaGet(
			points,
			job->pointsEA + i * sizeof(spray::Point),
			pointsCount * sizeof(spray::Point),
			context->dmaTag,
			0,
			0
		);
		cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);

		for (uint32_t j = 0; j < pointsCount; ++j)
			points[j].velocity += gravity * Scalar(points[j].inverseMass) * deltaTime;

		cellDmaPut(
			points,
			job->pointsEA + i * sizeof(spray::Point),
			pointsCount * sizeof(spray::Point),
			context->dmaTag,
			0,
			0
		);
		cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);
	}
}
