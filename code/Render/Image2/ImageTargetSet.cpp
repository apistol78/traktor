#include "Render/Image2/ImageTargetSet.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageTargetSet", ImageTargetSet, Object)

ImageTargetSet::ImageTargetSet(handle_t targetSetId, const RenderGraphTargetSetDesc& targetSetDesc)
:   m_targetSetId(targetSetId)
,   m_targetSetDesc(targetSetDesc)
{
}

handle_t ImageTargetSet::getTargetSetId() const
{
    return m_targetSetId;
}

const RenderGraphTargetSetDesc& ImageTargetSet::getTargetSetDesc() const
{
    return m_targetSetDesc;
}

	}
}
