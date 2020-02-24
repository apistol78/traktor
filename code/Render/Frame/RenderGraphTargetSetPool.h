#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Frame/RenderGraphTypes.h"

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

class IRenderSystem;
class IRenderTargetSet;

/*!
 * \ingroup Render
 */
class T_DLLCLASS RenderGraphTargetSetPool : public Object
{
	T_RTTI_CLASS;

public:
	struct Pool
	{
		RenderTargetSetCreateDesc rtscd;
		Ref< IRenderTargetSet > sharedDepthStencilTargetSet;
		RefArray< IRenderTargetSet > free;
		RefArray< IRenderTargetSet > acquired;
	};

	RenderGraphTargetSetPool(IRenderSystem* renderSystem);

	IRenderTargetSet* acquire(
		const RenderGraphTargetSetDesc& targetSetDesc,
		IRenderTargetSet* sharedDepthStencilTargetSet,
		int32_t referenceWidth,
		int32_t referenceHeight
	);

	void cleanup();

	void release(IRenderTargetSet* targetSet);

private:
	Ref< IRenderSystem > m_renderSystem;
	AlignedVector< Pool > m_pool;
	int32_t m_width;
	int32_t m_height;
};

	}
}
