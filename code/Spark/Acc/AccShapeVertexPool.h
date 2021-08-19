#pragma once

#include <list>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/VertexElement.h"

namespace traktor
{
	namespace render
	{

class Buffer;
class IRenderSystem;
class IVertexLayout;

	}

	namespace spark
	{

class AccShapeVertexPool : public Object
{
	T_RTTI_CLASS;

public:
	struct Range
	{
		render::Buffer* vertexBuffer = nullptr;
	};

	explicit AccShapeVertexPool(render::IRenderSystem* renderSystem, uint32_t frames);

	bool create(const AlignedVector< render::VertexElement >& vertexElements);

	void destroy();

	bool acquireRange(int32_t vertexCount, Range& outRange);

	void releaseRange(const Range& range);

	void cycleGarbage();

	const render::IVertexLayout* getVertexLayout() const { return m_vertexLayout; }

private:
	struct VertexRange
	{
		Ref< render::Buffer > vertexBuffer;
		int32_t vertexCount;
	};

	typedef std::list< VertexRange > vr_list_t;

	Ref< render::IRenderSystem > m_renderSystem;
	Ref< const render::IVertexLayout > m_vertexLayout;
	vr_list_t m_usedRanges;
	vr_list_t m_freeRanges;
	AlignedVector< vr_list_t > m_garbageRanges;
	uint32_t m_vertexSize = 0;
	uint32_t m_frame = 0;
};

	}
}

