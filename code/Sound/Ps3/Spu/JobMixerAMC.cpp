#include <cell/dma.h>
#include <cell/spurs/job_queue.h>
#include "Sound/Ps3/Spu/JobMC.h"

using namespace traktor;

void cellSpursJobQueueMain(CellSpursJobContext2* context, CellSpursJob256* job256)
{
	sound::JobMC* job = (sound::JobMC*)job256;

	static float lsb[1024] __attribute__((aligned(16)));
	static float rsb[1024] __attribute__((aligned(16)));

	cellDmaGet(
		lsb,
		job->lsbEA,
		job->count * sizeof(float),
		context->dmaTag,
		0,
		0
	);
	cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);

	cellDmaGet(
		rsb,
		job->rsbEA,
		job->count * sizeof(float),
		context->dmaTag,
		0,
		0
	);
	cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);

	for (uint32_t i = 0; i < job->count; ++i)
		lsb[i] += rsb[i] * job->factor;

	cellDmaPut(
		lsb,
		job->lsbEA,
		job->count * sizeof(float),
		context->dmaTag,
		0,
		0
	);
}
