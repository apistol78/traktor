#ifndef traktor_flash_AccShapeVertexPool_H
#define traktor_flash_AccShapeVertexPool_H

#include <list>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Flash/Acc/BlockList.h"

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
		int32_t offset;

		Range()
		:	vertexBuffer(0)
		,	offset(0)
		{
		}
	};

	AccShapeVertexPool(render::IRenderSystem* renderSystem);

	bool create();

	void destroy();

	bool acquireRange(int32_t vertexCount, Range& outRange);

	void releaseRange(const Range& range);

private:
	struct VertexPool
	{
		Ref< render::VertexBuffer > vertexBuffer;
		BlockList* blockList;
	};

	Ref< render::IRenderSystem > m_renderSystem;
	std::list< VertexPool > m_pools;
};

	}
}

#endif	// traktor_flash_AccShapeVertexPool_H
