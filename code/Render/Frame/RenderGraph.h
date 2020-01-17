#pragma once

#include <functional>
#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Types.h"
#include "Render/Frame/RenderPass.h"
#include "Render/Frame/RenderPassBuilder.h"
#include "Render/Frame/RenderPassResources.h"

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
class RenderContext;

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

/*! Render graph.
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
class T_DLLCLASS RenderGraph : public Object
{
	T_RTTI_CLASS;

public:
	/*! */
	explicit RenderGraph(IRenderSystem* renderSystem, int32_t width, int32_t height);

	/*! */
	void destroy();

	/*! Add render target definition.
	 *
	 * \param targetId Unique identifier of target.
	 * \param rtscd Render target set create description.
	 * \param rtas Optional information if size of target should be calculated from view size.
	 * \return True if target defined successfully.
	 */
	bool addRenderTarget(
		const handle_t targetId,
		const RenderTargetSetCreateDesc& rtscd,
		const RenderTargetAutoSize& rtas = RenderTargetAutoSize()
	);

	/*! Get render target from target identifier.
	 *
	 * \param targetId Unique identifier of target.
	 * \return Render target set.
	 */
	IRenderTargetSet* getRenderTarget(handle_t targetId) const;

	/*! */
	void addPass(const RenderPass::fn_setup_t& setup, const RenderPass::fn_build_t& build);

	/*! */
	bool validate();

	/*! */
	bool build(RenderContext* renderContext);

private:
	struct Target
	{
		RenderTargetSetCreateDesc rtscd;
		RenderTargetAutoSize rtas;
		Ref< IRenderTargetSet > rts[2];
	};

	Ref< IRenderSystem > m_renderSystem;
	SmallMap< handle_t, Target > m_targets;

	AlignedVector< RenderPass > m_passes;
	AlignedVector< uint32_t > m_order;

	int32_t m_width;
	int32_t m_height;
};

	}
}