#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Frame/IRenderGraph.h"

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

/*! Render graph implementation.
 * \ingroup Render
 */
class T_DLLCLASS RenderGraph : public IRenderGraph
{
    T_RTTI_CLASS;

public:
    explicit RenderGraph(IRenderSystem* renderSystem);

    virtual bool addRenderTarget(const std::wstring& targetId, const RenderTargetSetCreateDesc& rtscd, const RenderTargetAutoSize& rtas) override final;

    virtual IRenderTargetSet* getRenderTarget(const std::wstring& targetId) const override final;

	virtual RenderPass* addRenderPass(const std::wstring& passId) override final;

    virtual void removeRenderPass(const std::wstring& passId) override final;

    virtual bool validate(int32_t width, int32_t height) override final;

    virtual bool render(RenderContext* renderContext) override final;

private:
    struct Target
    {
        RenderTargetSetCreateDesc rtscd;
        RenderTargetAutoSize rtas;
        Ref< IRenderTargetSet > rts;
    };

    Ref< IRenderSystem > m_renderSystem;
    SmallMap< std::wstring, Target > m_targets;
    RefArray< const RenderPass > m_passes;
    AlignedVector< uint32_t > m_order;
    int32_t m_width;
    int32_t m_height;
};

    }
}