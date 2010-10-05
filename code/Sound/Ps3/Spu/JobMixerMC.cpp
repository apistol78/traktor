#include <cmath>
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>
#include "Core/Misc/Align.h"
#include "Sound/Ps3/Spu/JobMC.h"

using namespace traktor;

void cellSpursJobQueueMain(CellSpursJobContext2* context, CellSpursJob256* job256)
{
	sound::JobMC* job = (sound::JobMC*)job256;

	static float samples[4096] __attribute__((aligned(16)));

	for (uint32_t offset = 0; offset < job->mixer.count; offset += 4096)
	{
		uint32_t count = job->mixer.count - offset;
		if (count > 4096)
			count = 4096;
		else
			count = alignUp(count, 16);

		cellDmaGet(
			samples,
			job->mixer.rsbEA + offset,
			count * sizeof(float),
			context->dmaTag,
			0,
			0
		);
		cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);

		for (uint32_t i = 0; i < count; ++i)
			samples[i] *= job->mixer.factor;

		cellDmaPut(
			samples,
			job->mixer.lsbEA + offset,
			count * sizeof(float),
			context->dmaTag,
			0,
			0
		);
		cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);
	}
}
