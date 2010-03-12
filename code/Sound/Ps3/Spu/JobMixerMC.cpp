#include <cell/dma.h>
#include <cell/spurs/job_queue.h>

void cellSpursJobQueueMain(CellSpursJobContext2* context, CellSpursJob256* job)
{
	CellSpursJob128* job128 = (CellSpursJob128*)job;

	float samples[1024] __attribute__((aligned(16)));

	cellDmaGet(
		samples,
		job128->workArea.userData[1],
		job128->workArea.userData[2] * sizeof(float),
		context->dmaTag,
		0,
		0
	);
	cellSpursJobQueueDmaWaitTagStatusAll(1 << context->dmaTag);

	uint32_t fui = job128->workArea.userData[3];
	float factor = *(float*)&fui;

	for (uint32_t i = 0; i < job128->workArea.userData[2]; ++i)
		samples[i] *= factor;

	cellDmaPut(
		samples,
		job128->workArea.userData[0],
		job128->workArea.userData[2] * sizeof(float),
		context->dmaTag,
		0,
		0
	);
}
