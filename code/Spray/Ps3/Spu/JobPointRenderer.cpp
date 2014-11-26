#include <limits>
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>
#include "Core/Math/Const.h"
#include "Core/Math/Plane.h"
#include "Spray/Point.h"
#include "Spray/Vertex.h"
#include "Spray/Ps3/Spu/JobPointRenderer.h"

using namespace traktor;

#pragma pack(1)
struct Batch
{
	uint32_t count;
	float distance;
	uint32_t reserved[2];
};
#pragma pack()

void cellSpursJobQueueMain(CellSpursJobContext2* context, CellSpursJob256* job256)
{
	spray::JobPointRenderer* job = (spray::JobPointRenderer*)job256;

	const float c_extents[4][2] =
	{
		{ -1.0f,  1.0f },
		{  1.0f,  1.0f },
		{  1.0f, -1.0f },
		{ -1.0f, -1.0f }
	};

	enum { PointCount = 64 };

	static spray::Point points[PointCount] __attribute__((aligned(16)));
	static spray::EmitterVertex vertices[PointCount * 4] __attribute__((aligned(16)));
	static Batch batch __attribute__((aligned(16)));

	Plane cameraPlane __attribute__((aligned(16)));
	cameraPlane.set(
		job->data.cameraPlane[0],
		job->data.cameraPlane[1],
		job->data.cameraPlane[2],
		job->data.cameraPlane[3]
	);

	// Get batch descriptor.
	cellDmaGet(
		&batch,
		job->data.batchEA,
		sizeof(Batch),
		context->dmaTag,
		0,
		0
	);
	cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);

	uint32_t vertexOutOffset = 0;

	for (uint32_t i = 0; i < job->data.pointsCount; i += PointCount)
	{
		uint32_t pointsCount = job->data.pointsCount - i;
		if (pointsCount > PointCount)
			pointsCount = PointCount;

		// Get points.
		cellDmaGet(
			points,
			job->data.pointsEA + i * sizeof(spray::Point),
			pointsCount * sizeof(spray::Point),
			context->dmaTag,
			0,
			0
		);
		cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);

		// Generate vertex buffer data.
		uint32_t vertexOffset = 0;
		for (uint32_t j = 0; j < pointsCount; ++j)
		{
			const spray::Point& point = points[j];

			float distance = cameraPlane.distance(point.position);
			if (distance < job->data.cullNearDistance)
				continue;

			// Calculate alpha based on point age and distance from near culling plane.
			float age = clamp(point.age / point.maxAge, 0.0f, 1.0f);
			float middle = age - job->data.middleAge;
			float alpha = select(middle, 1.0f - middle / (1.0f - job->data.middleAge), age / job->data.middleAge);
			alpha *= min((distance - job->data.cullNearDistance) / job->data.fadeNearRange, 1.0f);
			if (alpha < FUZZY_EPSILON)
				continue;

			for (int j = 0; j < 4; ++j)
			{
				spray::EmitterVertex& vertex = vertices[vertexOffset++];

				point.position.storeAligned(vertex.positionAndOrientation);
				point.velocity.storeAligned(vertex.velocityAndRandom);
				point.color.storeAligned(vertex.colorAndAge);

				vertex.positionAndOrientation[3] = point.orientation;
				vertex.velocityAndRandom[3] = point.random;
				vertex.extentAlphaAndSize[0] = c_extents[j][0];
				vertex.extentAlphaAndSize[1] = c_extents[j][1];
				vertex.extentAlphaAndSize[2] = alpha;
				vertex.extentAlphaAndSize[3] = point.size;
				vertex.colorAndAge[3] = age;
			}

			batch.distance = min(batch.distance, distance);
			batch.count += 2;
		}

		// Write vertices.
		if (vertexOffset > 0)
		{
			cellDmaPut(
				vertices,
				job->data.vertexOutEA + vertexOutOffset,
				vertexOffset * sizeof(spray::EmitterVertex),
				context->dmaTag,
				0,
				0
			);
			//cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);
			vertexOutOffset += vertexOffset * sizeof(spray::EmitterVertex);
		}
	}

	// Put back batch descriptor.
	cellDmaPut(
		&batch,
		job->data.batchEA,
		sizeof(Batch),
		context->dmaTag,
		0,
		0
	);
	cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);
}
