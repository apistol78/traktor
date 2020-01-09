#pragma once

#include <string>
#include "Core/Object.h"
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

class RenderContext;
class RenderPass;

/*!
 * \ingroup Render
 */
struct RenderTargetAutoSize
{
    int32_t screenWidthDenom;
    int32_t screenHeightDenom;
    int32_t maxWidth;
    int32_t maxHeight;

    RenderTargetAutoSize()
    :   screenWidthDenom(0)
    ,   screenHeightDenom(0)
    ,   maxWidth(0)
    ,   maxHeight(0)
    {
    }
};

/*! Render graph interface.
 * \ingroup Render
 * 
 * A render graph describe all passes which is required
 * when rendering a frame. Since all passes is registered
 * beforehand passes can be organized to reduce
 * transitions of targets between passes.
 * 
 * External handlers are registered which
 * are called at appropriate times to render each pass.
 */
class T_DLLCLASS IRenderGraph : public Object
{
    T_RTTI_CLASS;

public:
    /*! Add render target definition.
     *
     * \param targetId Unique identifier of target.
     * \param rtscd Render target set create description.
     * \param rtas Optional information if size of target should be calculated from view size.
     * \return True if target defined successfully.
     */
    virtual bool addRenderTarget(
        const std::wstring& targetId,
        const RenderTargetSetCreateDesc& rtscd,
        const RenderTargetAutoSize& rtas = RenderTargetAutoSize()
    ) = 0;

    /*! Get render target from target identifier.
     *
     * \param targetId Unique identifier of target.
     * \return Render target set.
     */
    virtual IRenderTargetSet* getRenderTarget(const std::wstring& targetId) const = 0;

    /*! */
	virtual RenderPass* addRenderPass(const std::wstring& passId) = 0;

    /*! */
    virtual void removeRenderPass(const std::wstring& passId) = 0;

    /*! */
    virtual bool validate(int32_t width, int32_t height) = 0;

    /*! */
    virtual bool render(RenderContext* renderContext) = 0;
};

    }
}
