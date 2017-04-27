/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>
#include "Core/Math/Plane.h"
#include "Spray/Point.h"
#include "Spray/Ps3/Spu/JobModifierUpdate.h"

using namespace traktor;

void cellSpursJobQueueMain(CellSpursJobContext2* context, CellSpursJob256* job256)
{
	spray::JobModifierUpdate* job = (spray::JobModifierUpdate*)job256;

	static spray::Point points[1024];

	Scalar deltaTime(job->common.deltaTime);
	Plane plane(
		job->modifier.planeCollision.plane[0],
		job->modifier.planeCollision.plane[1],
		job->modifier.planeCollision.plane[2],
		job->modifier.planeCollision.plane[3]
	);

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
			Scalar rv = dot3(plane.normal(), points[j].velocity);
			if (rv >= 0.0f)
				continue;

			Scalar rd = plane.distance(points[j].position);
			if (rd >= points[j].size)
				continue;

			points[j].velocity = -reflect(points[j].velocity, plane.normal()) * Scalar(job->modifier.planeCollision.restitution);
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
