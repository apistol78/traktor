#ifndef traktor_render_RenderQueuePs4_H
#define traktor_render_RenderQueuePs4_H

#include <gnmx.h>
#include "Core/Ref.h"
#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;
class MemoryHeapObjectPs4;

/*! \brief Rendering queue.
 * \ingroup Render
 *
 * Each frame's queued rendering commands etc.
 */
class RenderQueuePs4 : public Object
{
	T_RTTI_CLASS;

public:
	enum EopState
	{
		EopsNotYet = 0,
		EopsFinished,
	};

	enum RenderContextState
	{
		RcsFree = 0,
		RcsInUse,
	};

	RenderQueuePs4(ContextPs4* context);

	bool create();

	void destroy();

	sce::Gnmx::GfxContext& getGfxContext() { return m_gfxContext; }

	volatile uint32_t* getEopLabel() { return m_eopLabel; }

	volatile uint32_t* getContextLabel() { return m_contextLabel; }

private:
	Ref< ContextPs4 > m_context;
	Ref< MemoryHeapObjectPs4 > m_memoryCue;
	Ref< MemoryHeapObjectPs4 > m_memoryDcb;
	Ref< MemoryHeapObjectPs4 > m_memoryCcb;
	Ref< MemoryHeapObjectPs4 > m_memoryLabels;
	sce::Gnmx::GfxContext m_gfxContext;
	volatile uint32_t* m_eopLabel;
	volatile uint32_t* m_contextLabel;
};

	}
}

#endif	// traktor_render_RenderQueuePs4_H
