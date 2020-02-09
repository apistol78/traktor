#include "Render/Image2/ImageTargetSet.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageTargetSet", ImageTargetSet, Object)

ImageTargetSet::ImageTargetSet(const RenderGraphTargetSetDesc& targetSetDesc)
:   m_targetSetDesc(targetSetDesc)
{
}

const RenderGraphTargetSetDesc& ImageTargetSet::getTargetSetDesc() const
{
    return m_targetSetDesc;
}

	}
}
