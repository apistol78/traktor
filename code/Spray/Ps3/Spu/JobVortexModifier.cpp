/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>
#include "Spray/Point.h"
#include "Spray/Ps3/Spu/JobModifierUpdate.h"

using namespace traktor;

void cellSpursJobQueueMain(CellSpursJobContext2* context, CellSpursJob256* job256)
{
	spray::JobModifierUpdate* job = (spray::JobModifierUpdate*)job256;

	static spray::Point points[1024];

	Scalar deltaTime(job->common.deltaTime);
	Vector4 center = job->common.transform.translation().xyz1();
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

		// Issue several DMA get tasks to get as many points as possible.
		for (uint32_t j = 0; j < pointsCount; j += 128)
		{
			uint32_t pointsDmaCount = pointsCount - j;
			if (pointsDmaCount > 128)
				pointsDmaCount = 128;

			cellDmaGet(
				&points[j],
				job->common.pointsEA + (i + j) * sizeof(spray::Point),
				pointsDmaCount * sizeof(spray::Point),
				context->dmaTag,
				0,
				0
			);
		}
		cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);

		for (uint32_t j = 0; j < pointsCount; ++j)
		{
			Vector4 pc = points[j].position - center;

			// Project onto plane.
			Scalar d = dot3(pc, axis);
			pc -= axis * d;

			// Calculate tangent vector.
			Scalar distance = pc.length();
			Vector4 n = pc / distance;
			Vector4 t = cross(axis, n).normalized();

			// Adjust velocity from this tangent.
			points[j].velocity += (
				t * tangentForce +
				n * (normalConstantForce + (distance - normalDistance) * normalDistanceForce)
			) * Scalar(points[j].inverseMass) * deltaTime;
		}

		// Issue DMA put tasks to update PPU points.
		for (uint32_t j = 0; j < pointsCount; j += 128)
		{
			uint32_t pointsDmaCount = pointsCount - j;
			if (pointsDmaCount > 128)
				pointsDmaCount = 128;

			cellDmaPut(
				&points[j],
				job->common.pointsEA + (i + j) * sizeof(spray::Point),
				pointsDmaCount * sizeof(spray::Point),
				context->dmaTag,
				0,
				0
			);
		}
		cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);
	}
}
