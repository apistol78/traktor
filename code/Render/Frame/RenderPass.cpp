#include "Render/Frame/RenderPass.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderPass", RenderPass, Object)

RenderPass::RenderPass()
:   m_name(nullptr)
,   m_refs(0)
{
}

    }
}
