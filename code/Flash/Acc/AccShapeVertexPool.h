#ifndef traktor_flash_AccShapeVertexPool_H
#define traktor_flash_AccShapeVertexPool_H

#include <list>
#include "Core/Object.h"
#include "Core/Ref.h"

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
#pragma pack(1)
	struct Vertex
	{
		float pos[2];
		float curvature[3];
		float texCoord[2];
		float texRect[4];
		uint8_t color[4];
	};
#pragma pack()

	struct Range
	{
		render::VertexBuffer* vertexBuffer;

		Range()
		:	vertexBuffer(0)
		{
		}
	};

	AccShapeVertexPool(render::IRenderSystem* renderSystem, uint32_t frames);

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
	vr_list_t m_usedRanges;
	vr_list_t m_freeRanges;
	std::vector< vr_list_t > m_garbageRanges;
	uint32_t m_frame;
};

	}
}

#endif	// traktor_flash_AccShapeVertexPool_H
