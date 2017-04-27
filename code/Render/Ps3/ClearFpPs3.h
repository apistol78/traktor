/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ClearFpPs3_H
#define traktor_render_ClearFpPs3_H

#include "Render/Ps3/PlatformPs3.h"

namespace traktor
{
	namespace render
	{

class MemoryHeap;
class MemoryHeapObject;
class StateCachePs3;

/*! \brief Clear FP targets helper.
 *
 * As PS3 doesn't support clearing FP targets
 * we clear targets by drawing a solid quad
 * covering entire target.
 */
class ClearFpPs3
{
public:
	ClearFpPs3();

	virtual ~ClearFpPs3();

	bool create(MemoryHeap* memoryHeap);

	void clear(StateCachePs3& stateCache, const float color[4]);

private:
	MemoryHeapObject* m_quadBuffer;
	uint32_t m_quadBufferOffset;
	CGprogram m_clearVertexProgram;
	CGprogram m_clearFragmentProgram;
	void* m_clearVertexProgramUcode;
	MemoryHeapObject* m_clearFragmentProgramUcode;
	uint32_t m_clearPositionIndex;
};

	}
}

#endif	// traktor_render_ClearFpPs3_H
