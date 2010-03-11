#ifndef traktor_render_RenderContext_H
#define traktor_render_RenderContext_H

#include <map>
#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Matrix44.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/ShaderParameters.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderView;

/*! \brief Render block type.
 * \ingroup Render
 */
enum RenderBlockType
{
	RfOpaque = 1,
	RfAlphaBlend = 2,
	RfOverlay = 4,
	RfAll = (RfOpaque | RfAlphaBlend | RfOverlay)
};

/*! \brief Deferred render context.
 * \ingroup Render
 *
 * A render context is used to defer rendering in a
 * multi-threaded renderer.
 */
class T_DLLCLASS RenderContext : public Object
{
	T_RTTI_CLASS;

public:
#if defined(WINCE)
	enum { DefaultHeapSize = 512 * 1024 };
#else
	enum { DefaultHeapSize = 4 * 1024 * 1024 };
#endif

	RenderContext(uint32_t heapSize = DefaultHeapSize);

	virtual ~RenderContext();

	/*! \brief Allocate a unaligned block of memory from context's heap. */
	void* alloc(int blockSize);

	/*! \brief Allocate a aligned block of memory from context's heap. */
	void* alloc(int blockSize, int align);

	/*! \brief Allocate object from context's heap.
	 *
	 * \note Object's destructor won't be called when
	 * heap is flushed.
	 */
	template < typename ObjectType >
	ObjectType* alloc()
	{
		ObjectType* object = reinterpret_cast< ObjectType* >(alloc(sizeof(ObjectType), alignOf< ObjectType >()));
		new (object) ObjectType();
		return object;
	}

	/*! \brief Allocate named object from context's heap. */
	template < typename ObjectType >
	ObjectType* alloc(const char* const name)
	{
		ObjectType* object = alloc< ObjectType >();
#if defined T_USE_RENDERBLOCK_NAME
		object->name = name;
#endif
		return object;
	}

	/*! \brief Enqueue a render block in context. */
	void draw(uint32_t type, RenderBlock* renderBlock);

	/*! \brief Render blocks. */
	void render(IRenderView* renderView, uint32_t flags) const;

	/*! \brief Flush blocks. */
	void flush();

	inline uint32_t getAllocatedSize() const { return uint32_t(m_heapPtr - &m_heap[0]); }

private:
	uint8_t* m_heap;
	uint8_t* m_heapEnd;
	uint8_t* m_heapPtr;
	std::vector< RenderBlock* > m_renderQueue[3];
};

	}
}

#endif	// traktor_render_RenderContext_H
