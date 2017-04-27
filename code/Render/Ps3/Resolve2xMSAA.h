/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Resolve2xMSAA_H
#define traktor_render_Resolve2xMSAA_H

#include "Render/Ps3/PlatformPs3.h"

namespace traktor
{
	namespace render
	{

class StateCachePs3;
class MemoryHeap;
class MemoryHeapObject;

class Resolve2xMSAA
{
public:
	Resolve2xMSAA();

	virtual ~Resolve2xMSAA();

	bool create(MemoryHeap* memoryHeap);

	void resolve(StateCachePs3& stateCache, const CellGcmTexture* resolvedTexture, const CellGcmTexture* sourceTexture);

private:
	MemoryHeapObject* m_quadBuffer[4];
	CGprogram m_vertexProgram;
	CGprogram m_fragmentProgram;
	void* m_vertexProgramUcode;
	MemoryHeapObject* m_fragmentProgramUcode;
	int32_t m_positionIndex;
	int32_t m_tex0Index;
	int32_t m_texture0Index;
	int32_t m_texture1Index;
	int32_t m_lastWidth;
	int32_t m_lastHeight;
	int32_t m_quadBufferCounter;
};

	}
}

#endif	// traktor_render_Resolve2xMSAA_H
