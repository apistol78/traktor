#include <cmath>
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>
#include "Sound/Ps3/Spu/JobMC.h"

using namespace traktor;

void cellSpursJobQueueMain(CellSpursJobContext2* context, CellSpursJob256* job256)
{
	sound::JobMC* job = (sound::JobMC*)job256;

	static float lsb[1024 + 512] __attribute__((aligned(16)));
	static float rsb[1024 + 512] __attribute__((aligned(16)));

	cellDmaGet(
		rsb,
		job->mixer.rsbEA,
		job->mixer.rcount * sizeof(float),
		context->dmaTag,
		0,
		0
	);
	cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);

	for (uint32_t s = 0; s < job->mixer.count; ++s)
	{
		uint32_t s0 = (s * job->mixer.rcount) / job->mixer.count;
		lsb[s] = rsb[s0] * job->mixer.factor;
	}

	cellDmaPut(
		lsb,
		job->mixer.lsbEA,
		job->mixer.count * sizeof(float),
		context->dmaTag,
		0,
		0
	);
}
