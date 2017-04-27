/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"

#define SM_SPURS_SPU_COUNT 4
#define SM_SPURS_QUEUE_DEPTH(submitCount) (16 * (submitCount))

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.SpursJobQueue", SpursJobQueue, Object)

void SpursJobQueue::destroy()
{
	int ret;

	if (m_jobQueuePort)
	{
		ret = cellSpursJobQueuePortSync(m_jobQueuePort);
		T_FATAL_ASSERT_M (ret == CELL_OK, L"cellSpursJobQueuePortSync failed");
		
		ret = cellSpursJobQueuePortFinalize(m_jobQueuePort);
		T_FATAL_ASSERT_M (ret == CELL_OK, L"cellSpursJobQueuePortFinalize failed");

		Alloc::freeAlign(m_jobQueuePort);
		m_jobQueuePort = 0;
	}

	if (m_jobQueue)
	{
		int exitCode;
		
		ret = cellSpursShutdownJobQueue(m_jobQueue);
		T_FATAL_ASSERT_M (ret == CELL_OK, L"cellSpursShutdownJobQueue failed");
		
		ret = cellSpursJoinJobQueue(m_jobQueue, &exitCode);
		T_FATAL_ASSERT_M (ret == CELL_OK, L"cellSpursJoinJobQueue failed");

		Alloc::freeAlign(m_jobQueue);
		m_jobQueue = 0;
	}

	if (m_descriptorBuffer)
	{
		Alloc::freeAlign(m_descriptorBuffer);
		m_descriptorBuffer = 0;
	}

	if (m_commandQueue)
	{
		Alloc::freeAlign(m_commandQueue);
		m_commandQueue = 0;
	}

	m_spurs = 0;
}

bool SpursJobQueue::push(CellSpursJobHeader* job)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (timeout < 0);
	int res = cellSpursJobQueuePortSync(m_jobQueuePort);
	return res == CELL_OK;
}

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

bool SpursJobQueue::create(uint32_t descriptorSize, uint32_t submitCount, int priority)
{
	int32_t res;

	T_FATAL_ASSERT_M (
		descriptorSize == 64 ||
		(descriptorSize & ~127UL) == descriptorSize,
		L"Invalid size of descriptor"
	);

	m_commandQueue = (uint64_t*)Alloc::acquireAlign(
		CELL_SPURS_JOBQUEUE_SIZE_COMMAND_BUFFER(SM_SPURS_QUEUE_DEPTH(submitCount)),
		CELL_SPURS_JOBQUEUE_COMMAND_BUFFER_ALIGN,
		T_FILE_LINE
	);

	m_jobQueue = (CellSpursJobQueue*)Alloc::acquireAlign(
		CELL_SPURS_JOBQUEUE_SIZE,
		CELL_SPURS_JOBQUEUE_ALIGN,
		T_FILE_LINE
	);

	CellSpursJobQueueAttribute attributeJobQueue;
	cellSpursJobQueueAttributeInitialize(&attributeJobQueue);
	cellSpursJobQueueAttributeSetMaxGrab(&attributeJobQueue, 4);

	const uint8_t priorityTable[5][8] =
	{
		{ 15, 15, 15, 15, 15, 15, 15, 15 },	// Lowest
		{ 12, 12, 12, 12, 12, 12, 12, 12 },
		{ 8, 8, 8, 8, 8, 8, 8, 8 },
		{ 4, 4, 4, 4, 4, 4, 4, 4 },
		{ 1, 1, 1, 1, 1, 1, 1, 1 }	// Highest
	};

	res = cellSpursCreateJobQueue(
		m_spurs,
		m_jobQueue,
		&attributeJobQueue,
		"Traktor Spurs JobQueue",
		m_commandQueue,
		SM_SPURS_QUEUE_DEPTH(submitCount),
		SM_SPURS_SPU_COUNT,
		priorityTable[priority]
	);
	if (res != CELL_OK)
		return false;

	m_descriptorBuffer = (CellSpursJobHeader*)Alloc::acquireAlign(
		descriptorSize * submitCount,
		128,
		T_FILE_LINE
	);

	m_descriptorSize = descriptorSize;

	m_jobQueuePort = (CellSpursJobQueuePort*)Alloc::acquireAlign(
		CELL_SPURS_JOBQUEUE_PORT_SIZE,
		CELL_SPURS_JOBQUEUE_PORT_ALIGN,
		T_FILE_LINE
	);

	res = cellSpursJobQueuePortInitializeWithDescriptorBuffer(
		m_jobQueuePort,
		m_jobQueue,
		m_descriptorBuffer,
		descriptorSize,
		submitCount,
		true
	);
	if (res != CELL_OK)
		return false;

	return true;
}

}
