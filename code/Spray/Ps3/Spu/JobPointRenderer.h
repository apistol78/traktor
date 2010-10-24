#ifndef traktor_spray_JobPointRenderer_H
#define traktor_spray_JobPointRenderer_H

#include <cell/spurs/job_descriptor.h>

namespace traktor
{
	namespace spray
	{

struct JobPointRendererResult
{
	uint32_t vertexOffset;
};
		
struct JobPointRenderer
{
	CellSpursJobHeader header;

	struct Data
	{
		float cameraPlane[4];
		float cullNearDistance;
		float fadeNearRange;
		float middleAge;
		uintptr_t pointsEA;
		uint32_t pointsCount;
		uintptr_t vertexOutEA;
		uintptr_t batchEA;
	}
	data;

	uint8_t pad[128 - sizeof(CellSpursJobHeader) - sizeof(Data)];
};

	}
}

#endif	// traktor_spray_JobPointRenderer_H
