#ifndef traktor_render_RenderContext_H
#define traktor_render_RenderContext_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/ShaderParameters.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderView;

/*! \brief Deferred render context.
 * \ingroup Render
 *
 * A render context is used to defer rendering in a
 * multi-threaded renderer.
 */
class T_DLLCLASS RenderContext : public Object
{
	T_RTTI_CLASS(RenderContext)

public:
	enum
	{
		RfOpaque = 1,
		RfAlphaBlend = 2
	};

	RenderContext(IRenderView* renderView, uint32_t heapSize = 4 * 1024 * 1024);

	virtual ~RenderContext();

	/*! \brief Allocate a block of memory from context's heap. */
	void* alloc(int blockSize);

	/*! \brief Allocate object from context's heap.
	 *
	 * \note Object's destructor won't be called when
	 * heap is flushed.
	 */
	template < typename ObjectType >
	ObjectType* alloc()
	{
		ObjectType* object = reinterpret_cast< ObjectType* >(alloc(sizeof(ObjectType)));
		new (object) ObjectType();
		return object;
	}

	/*! \brief Enqueue a render block in context. */
	void draw(RenderBlock* renderBlock);

	/*! \brief Render blocks. */
	void render(uint32_t flags) const;

	/*! \brief Flush blocks. */
	void flush();

	inline IRenderView* getRenderView() const { return m_renderView; }

	inline uint32_t getAllocatedSize() const { return uint32_t(m_heapPtr - &m_heap[0]); }

private:
	Ref< IRenderView > m_renderView;
	uint8_t* m_heap;
	uint8_t* m_heapEnd;
	uint8_t* m_heapPtr;
	std::vector< RenderBlock* > m_renderQueue[2];
};

	}
}

#endif	// traktor_render_RenderContext_H
