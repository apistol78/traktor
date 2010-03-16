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

	Scalar deltaTime(job->common.deltaTime);
	Vector4 center = job->common.transform.translation();
	Vector4 axis(job->modifier.vortex.axis);
	Scalar tangentForce(job->modifier.vortex.tangentForce);
	Scalar normalDistance(job->modifier.vortex.normalDistance);
	Scalar normalDistanceForce(job->modifier.vortex.normalDistanceForce);
	Scalar normalConstantForce(job->modifier.vortex.normalConstantForce);

	for (uint32_t i = 0; i < job->common.pointsCount; i += sizeof_array(points))
	{
		uint32_t pointsCount = job->common.pointsCount - i;
		if (pointsCount > sizeof_array(points))
			pointsCount = sizeof_array(points);

		cellDmaGet(
			points,
			job->common.pointsEA + i * sizeof(spray::Point),
			pointsCount * sizeof(spray::Point),
			context->dmaTag,
			0,
			0
		);
		cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);

		for (uint32_t j = 0; j < pointsCount; ++j)
		{
			Vector4 pc = points[i].position - center;

			// Project onto plane.
			Scalar d = dot3(pc, axis);
			pc -= axis * d;

			// Calculate tangent vector.
			Scalar distance = pc.length();
			Vector4 n = pc / distance;
			Vector4 t = cross(axis, n).normalized();

			// Adjust velocity from this tangent.
			points[i].velocity += (
				t * tangentForce +
				n * (normalConstantForce + (distance - normalDistance) * normalDistanceForce)
			) * Scalar(points[i].inverseMass) * deltaTime;
		}

		cellDmaPut(
			points,
			job->common.pointsEA + i * sizeof(spray::Point),
			pointsCount * sizeof(spray::Point),
			context->dmaTag,
			0,
			0
		);
		cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);
	}
}
