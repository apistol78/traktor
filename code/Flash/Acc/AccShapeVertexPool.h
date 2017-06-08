/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AccShapeVertexPool_H
#define traktor_flash_AccShapeVertexPool_H

#include <list>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/VertexElement.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class VertexBuffer;

	}

	namespace flash
	{

class AccShapeVertexPool : public Object
{
	T_RTTI_CLASS;

public:
	struct Range
	{
		render::VertexBuffer* vertexBuffer;

		Range()
		:	vertexBuffer(0)
		{
		}
	};

	AccShapeVertexPool(render::IRenderSystem* renderSystem, uint32_t frames, const std::vector< render::VertexElement >& vertexElements);

	bool create();

	void destroy();

	bool acquireRange(int32_t vertexCount, Range& outRange);

	void releaseRange(const Range& range);

	void cycleGarbage();

private:
	struct VertexRange
	{
		Ref< render::VertexBuffer > vertexBuffer;
		int32_t vertexCount;
	};

	typedef std::list< VertexRange > vr_list_t;

	Ref< render::IRenderSystem > m_renderSystem;
	std::vector< render::VertexElement > m_vertexElements;
	vr_list_t m_usedRanges;
	vr_list_t m_freeRanges;
	AlignedVector< vr_list_t > m_garbageRanges;
	uint32_t m_frame;
};

	}
}

#endif	// traktor_flash_AccShapeVertexPool_H
