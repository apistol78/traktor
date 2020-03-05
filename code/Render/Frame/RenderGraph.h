#pragma once

#include <functional>
#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/StaticVector.h"
#include "Render/Frame/RenderGraphTypes.h"
#include "Render/Frame/RenderPass.h"

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
class RenderGraphTargetSetPool;

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
	explicit RenderGraph(IRenderSystem* renderSystem);

	/*! */
	void destroy();

	/*! Add transient target set.
	 *
	 * \param targetSetDesc Render target set create description.
	 * \param sharedDepthStencil Share depth/stencil with target set.
	 * \param sizeReferenceTargetSetId Target to get reference size from when determine target set.
	 * \return Opaque handle of transient target set.
	 */
	handle_t addTargetSet(
		const RenderGraphTargetSetDesc& targetSetDesc,
		IRenderTargetSet* sharedDepthStencil = nullptr,
		handle_t sizeReferenceTargetSetId = 0
	);

	/*! Add external target set.
	 *
	 * \param targetSet Render target set.
	 * \return Opaque handle of target set.
	 */
	handle_t addTargetSet(IRenderTargetSet* targetSet);

	/*! Get transient target set from target identifier.
	 *
	 * \param targetSetId Unique identifier of target.
	 * \return Render target set.
	 */
	IRenderTargetSet* getTargetSet(handle_t targetSetId) const;

	/*! Add render pass to graph.
	 *
	 * \param pass Render pass to add.
	 */
	void addPass(const RenderPass* pass);

	/*! */
	bool validate(int32_t width, int32_t height);

	/*! */
	bool build(RenderContext* renderContext);

	/*! */
	void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const;

private:
	struct Target
	{
		RenderGraphTargetSetDesc targetSetDesc;
		Ref< IRenderTargetSet > sharedDepthStencilTargetSet;
		Ref< IRenderTargetSet > rts;
		handle_t sizeReferenceTargetSetId;
		bool transient;
	};

	Ref< RenderGraphTargetSetPool > m_pool;
	SmallMap< handle_t, Target > m_targets;
	RefArray< const RenderPass > m_passes;
	StaticVector< uint32_t, 256 > m_order;
	handle_t m_nextTargetSetId;

	void traverse(int32_t index, const std::function< void(int32_t) >& fn) const;
};

	}
}