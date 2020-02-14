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
		RenderGraphTargetSetDesc targetSetDesc;
		Ref< IRenderTargetSet > sharedDepthStencilTargetSet;
		RefArray< IRenderTargetSet > free;
		RefArray< IRenderTargetSet > acquired;
	};

	RenderGraphTargetSetPool(IRenderSystem* renderSystem);

	bool validate(int32_t width, int32_t height);

	IRenderTargetSet* acquire(const RenderGraphTargetSetDesc& targetSetDesc, IRenderTargetSet* sharedDepthStencilTargetSet);

	void release(IRenderTargetSet* targetSet);

private:
	Ref< IRenderSystem > m_renderSystem;
	AlignedVector< Pool > m_pool;
	int32_t m_width;
	int32_t m_height;
};

	}
}
