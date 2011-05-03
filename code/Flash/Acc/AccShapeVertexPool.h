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
	AccShapeVertexPool(render::IRenderSystem* renderSystem);

	bool create();

	void destroy();

	render::VertexBuffer* acquireVertexBuffer(int32_t triangleCount);

	void releaseVertexBuffer(render::VertexBuffer* vertexBuffer);

private:
	struct PoolEntry
	{
		Ref< render::VertexBuffer > vertexBuffer;
		int32_t triangleCount;
	};

	Ref< render::IRenderSystem > m_renderSystem;
	std::list< PoolEntry > m_freeBuffers;
	std::list< PoolEntry > m_usedBuffers;
};

	}
}

#endif	// traktor_flash_AccShapeVertexPool_H
