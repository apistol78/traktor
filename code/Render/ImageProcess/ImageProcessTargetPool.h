#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"

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

/*! Pool of render targets.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessTargetPool : public Object
{
	T_RTTI_CLASS;

public:
	struct Pool
	{
		RenderTargetSetCreateDesc rtscd;
		RefArray< IRenderTargetSet > free;
		RefArray< IRenderTargetSet > acquired;
	};

	ImageProcessTargetPool(IRenderSystem* renderSystem);

	IRenderTargetSet* acquireTarget(const RenderTargetSetCreateDesc& rtscd);

	void releaseTarget(const RenderTargetSetCreateDesc& rtscd, IRenderTargetSet* rts);

private:
	Ref< IRenderSystem > m_renderSystem;
	AlignedVector< Pool > m_pool;
};

	}
}

