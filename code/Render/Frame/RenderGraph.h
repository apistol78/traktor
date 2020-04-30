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
	struct Target
	{
		const wchar_t* name;
		RenderGraphTargetSetDesc targetSetDesc;
		Ref< IRenderTargetSet > sharedDepthStencilTargetSet;
		Ref< IRenderTargetSet > rts;
		handle_t sizeReferenceTargetSetId;
		int32_t referenceCount;
		bool storeDepth;
		bool transient;

		Target()
		:	name(nullptr)
		,	sizeReferenceTargetSetId(0)
		,	referenceCount(0)
		,	storeDepth(false)
		,	transient(false)
		{
		}
	};

	/*! */
	explicit RenderGraph(IRenderSystem* renderSystem);

	/*! */
	void destroy();

	/*! Add transient target set.
	 *
	 * \param name Name of target set, used for debugging only.
	 * \param targetSetDesc Render target set create description.
	 * \param sharedDepthStencil Share depth/stencil with target set.
	 * \param sizeReferenceTargetSetId Target to get reference size from when determine target set.
	 * \return Opaque handle of transient target set.
	 */
	handle_t addTransientTargetSet(
		const wchar_t* const name,
		const RenderGraphTargetSetDesc& targetSetDesc,
		IRenderTargetSet* sharedDepthStencil = nullptr,
		handle_t sizeReferenceTargetSetId = 0
	);

	/*! Add persistent target set.
	 *
	 * \param name Name of target set, used for debugging only.
	 * \param targetSetDesc Render target set create description.
	 * \param sharedDepthStencil Share depth/stencil with target set.
	 * \param sizeReferenceTargetSetId Target to get reference size from when determine target set.
	 * \return Opaque handle of transient target set.
	 */
	handle_t addPersistentTargetSet(
		const wchar_t* const name,
		const RenderGraphTargetSetDesc& targetSetDesc,
		IRenderTargetSet* sharedDepthStencil = nullptr,
		handle_t sizeReferenceTargetSetId = 0
	);

	/*! Add external target set.
	 *
	 * \param name Name of target set, used for debugging only.
	 * \param targetSet Render target set.
	 * \return Opaque handle of target set.
	 */
	handle_t addExternalTargetSet(const wchar_t* const name, IRenderTargetSet* targetSet);

	/*! Find target ID by name.
	 *
	 * \param name Name of target set, used for debugging only.
	 * \return ID of target set.
	 */
	handle_t findTargetByName(const wchar_t* const name) const;

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
	bool validate();

	/*! */
	bool build(RenderContext* renderContext, int32_t width, int32_t height);

	/*! */
	const SmallMap< handle_t, Target >& getTargets() const;

private:
	Ref< RenderGraphTargetSetPool > m_pool;
	SmallMap< handle_t, Target > m_targets;
	RefArray< const RenderPass > m_passes;
	StaticVector< uint32_t, 512 > m_order;
	handle_t m_nextTargetSetId;

	void traverse(int32_t index, const std::function< void(int32_t) >& fn) const;
};

	}
}