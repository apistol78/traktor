#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Frame/RenderGraphTypes.h"

namespace traktor::render
{

class IRenderSystem;
class IRenderTargetSet;

/*!
 * \ingroup Render
 */
class RenderGraphTargetSetPool : public Object
{
	T_RTTI_CLASS;

public:
	explicit RenderGraphTargetSetPool(IRenderSystem* renderSystem);

	void destroy();

	/*! Acquire target from pool.
	 *
	 * \param name Name of target, useful for debugging etc.
	 * \param targetSetDesc Description of target required.
	 * \param sharedDepthStencilTargetSet Optional shared depth/stencil target set.
	 * \param referenceWidth Reference width of target required.
	 * \param referenceHeight Reference height of target required.
	 * \param persistentHandle Persistent handle; used to track persistent targets in pool, 0 means not persistent target.
	 */
	IRenderTargetSet* acquire(
		const wchar_t* name,
		const RenderGraphTargetSetDesc& targetSetDesc,
		IRenderTargetSet* sharedDepthStencilTargetSet,
		int32_t referenceWidth,
		int32_t referenceHeight,
		uint32_t multiSample,
		handle_t persistentHandle
	);

	/*! */
	void release(IRenderTargetSet* targetSet);

	/*! */
	void cleanup();

private:
	struct Target
	{
		Ref< IRenderTargetSet > rts;
		int32_t unused;
	};

	struct Pool
	{
		// Pool identification.
		RenderTargetSetCreateDesc rtscd;
		Ref< IRenderTargetSet > sharedDepthStencilTargetSet;
		handle_t persistentHandle;

		// Pool targets.
		AlignedVector< Target > free;
		RefArray< IRenderTargetSet > acquired;
	};

	Ref< IRenderSystem > m_renderSystem;
	AlignedVector< Pool > m_pool;
};

}
