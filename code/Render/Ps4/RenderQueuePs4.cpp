/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/PS4/ContextPs4.h"
#include "Render/PS4/MemoryHeapObjectPs4.h"
#include "Render/PS4/MemoryHeapPs4.h"
#include "Render/PS4/RenderQueuePs4.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_cueRingEntries = 64;
const uint32_t c_dcbSize = 2 * 1024 * 1024;
const uint32_t c_ccbSize = 2 * 1024 * 1024;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderQueuePs4", RenderQueuePs4, Object)

RenderQueuePs4::RenderQueuePs4(ContextPs4* context)
:	m_context(context)
,	m_eopLabel(0)
,	m_contextLabel(0)
{
}

bool RenderQueuePs4::create()
{
	m_memoryCue = m_context->getHeapGarlic()->alloc(
		sce::Gnmx::ConstantUpdateEngine::computeHeapSize(c_cueRingEntries),
		sce::Gnm::kAlignmentOfBufferInBytes,
		true
	);
	if (!m_memoryCue)
		return false;

	m_memoryDcb = m_context->getHeapOnion()->alloc(c_dcbSize, sce::Gnm::kAlignmentOfBufferInBytes, true);
	if (!m_memoryDcb)
		return false;

	m_memoryCcb = m_context->getHeapOnion()->alloc(c_ccbSize, sce::Gnm::kAlignmentOfBufferInBytes, true);
	if (!m_memoryCcb)
		return false;

	m_memoryLabels = m_context->getHeapOnion()->alloc(2 * 4, 8, true);
	if (!m_memoryLabels)
		return false;

	m_gfxContext.init(
		m_memoryCue->getPointer(),
		c_cueRingEntries,
		m_memoryDcb->getPointer(),
		c_dcbSize,
		m_memoryCcb->getPointer(),
		c_ccbSize
	);

	m_eopLabel = static_cast< uint32_t* >(m_memoryLabels->getPointer());
	m_contextLabel = static_cast< uint32_t* >(m_memoryLabels->getPointer()) + 1;

	*m_eopLabel = EopsFinished;
	*m_contextLabel = RcsFree;

	return true;
}

void RenderQueuePs4::destroy()
{
	if (m_memoryLabels)
	{
		m_memoryLabels->free();
		m_memoryLabels = 0;
	}

	if (m_memoryCcb)
	{
		m_memoryCcb->free();
		m_memoryCcb = 0;
	}

	if (m_memoryDcb)
	{
		m_memoryDcb->free();
		m_memoryDcb = 0;
	}

	if (m_memoryCue)
	{
		m_memoryCue->free();
		m_memoryCue = 0;
	}
}

	}
}
