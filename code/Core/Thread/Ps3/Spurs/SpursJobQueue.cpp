#include "Core/Memory/Alloc.h"
#include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"

#define SM_SPURS_SPU_COUNT 4

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.SpursJobQueue", SpursJobQueue, Object)

SpursJobQueue::SpursJobQueue(CellSpurs* spurs)
:	m_spurs(spurs)
,	m_commandQueue(0)
,	m_descriptorBuffer(0)
,	m_descriptorSize(0)
,	m_jobQueue(0)
,	m_jobQueuePort(0)
{
}

SpursJobQueue::~SpursJobQueue()
{
	destroy();
}

bool SpursJobQueue::create(uint32_t descriptorSize, uint32_t submitCount)
{
	m_commandQueue = (uint64_t*)Alloc::acquireAlign(
		CELL_SPURS_JOBQUEUE_SIZE_COMMAND_BUFFER(/*depth*/16),
		CELL_SPURS_JOBQUEUE_COMMAND_BUFFER_ALIGN
	);

	m_jobQueue = (CellSpursJobQueue*)Alloc::acquireAlign(
		CELL_SPURS_JOBQUEUE_SIZE,
		CELL_SPURS_JOBQUEUE_ALIGN
	);

	CellSpursJobQueueAttribute attributeJobQueue;
	cellSpursJobQueueAttributeInitialize(&attributeJobQueue);
	cellSpursJobQueueAttributeSetMaxGrab(&attributeJobQueue, 4);

	uint8_t priorityTable[] = { 8, 8, 8, 8, 8, 8, 8, 8 };

	cellSpursCreateJobQueue(
		m_spurs,
		m_jobQueue,
		&attributeJobQueue,
		"Traktor Spurs JobQueue",
		m_commandQueue,
		/*depth*/16,
		SM_SPURS_SPU_COUNT,
		priorityTable
	);

	m_descriptorBuffer = (CellSpursJobHeader*)Alloc::acquireAlign(
		descriptorSize * submitCount,
		128
	);

	m_descriptorSize = descriptorSize;

	m_jobQueuePort = (CellSpursJobQueuePort*)Alloc::acquireAlign(
		CELL_SPURS_JOBQUEUE_PORT_SIZE,
		CELL_SPURS_JOBQUEUE_PORT_ALIGN
	);

	cellSpursJobQueuePortInitializeWithDescriptorBuffer(
		m_jobQueuePort,
		m_jobQueue,
		m_descriptorBuffer,
		descriptorSize,
		submitCount,
		true
	);

	return true;
}

void SpursJobQueue::destroy()
{
	if (m_jobQueuePort)
	{
		cellSpursJobQueuePortFinalize(m_jobQueuePort);
		Alloc::freeAlign(m_jobQueuePort);
		m_jobQueuePort = 0;
	}

	if (m_jobQueue)
	{
		int exitCode;
		cellSpursShutdownJobQueue(m_jobQueue);
		cellSpursJoinJobQueue(m_jobQueue, &exitCode);
		Alloc::freeAlign(m_jobQueue);
		m_jobQueue = 0;
	}

	if (m_descriptorBuffer)
	{
		Alloc::freeAlign(m_descriptorBuffer);
		m_descriptorBuffer = 0;
	}

	if (m_jobQueuePort)
	{
		Alloc::freeAlign(m_jobQueuePort);
		m_jobQueuePort = 0;
	}

	m_spurs = 0;
}

bool SpursJobQueue::push(CellSpursJobHeader* job)
{
	int res = cellSpursJobQueuePortCopyPush(
		m_jobQueuePort,
		job,
		m_descriptorSize,
		true
	);
	return res == CELL_OK;
}

bool SpursJobQueue::wait(int32_t timeout)
{
	T_ASSERT (timeout < 0);
	int res = cellSpursJobQueuePortSync(m_jobQueuePort);
	return res == CELL_OK;
}

}
