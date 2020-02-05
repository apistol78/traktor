#pragma once

#include <functional>
#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Render/Types.h"
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

/*!
 * \ingroup Render
 */
struct RenderGraphTargetSetDesc
{
	enum { MaxColorTargets = 8 };

	int32_t count;
	int32_t width;
	int32_t height;
	int32_t screenWidthDenom;
	int32_t screenHeightDenom;
	int32_t maxWidth;
	int32_t maxHeight;
	bool createDepthStencil;
	bool usingDepthStencilAsTexture;
	bool generateMips;
	TextureFormat colorFormats[MaxColorTargets];

	RenderGraphTargetSetDesc()
	:	count(0)
	,	width(0)
	,	height(0)
	,	screenWidthDenom(0)
	,	screenHeightDenom(0)
	,	maxWidth(0)
	,	maxHeight(0)
	,	createDepthStencil(false)
	,	usingDepthStencilAsTexture(false)
	,	generateMips(false)
	{
		for (int32_t i = 0; i < sizeof_array(colorFormats); ++i)
			colorFormats[i] = TfInvalid;
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

	/*! Add target definition.
	 *
	 * \param name Debug name of target.
	 * \param targetId Unique identifier of target.
	 * \param targetSetDesc Render target set create description.
	 * \return True if target defined successfully.
	 */
	bool addTargetSet(
		const wchar_t* const name,
		const handle_t targetSetId,
		const RenderGraphTargetSetDesc& targetSetDesc
	);

	/*! Get target set from target identifier.
	 *
	 * \param targetSetId Unique identifier of target.
	 * \param history Get history target.
	 * \return Render target set.
	 */
	IRenderTargetSet* getTargetSet(handle_t targetSetId, bool history = false) const;

	/*! Add render pass to graph.
	 *
	 * \param pass Render pass to add.
	 */
	void addPass(const RenderPass* pass);

	/*! */
	bool validate();

	/*! */
	bool build(RenderContext* renderContext);

	/*! */
	void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const;

private:
	struct Target
	{
		const wchar_t* name;
		RenderGraphTargetSetDesc targetSetDesc;
		Ref< IRenderTargetSet > rts[2];
	};

	Ref< IRenderSystem > m_renderSystem;
	SmallMap< handle_t, Target > m_targets;
	RefArray< const RenderPass > m_passes;
	AlignedVector< uint32_t > m_order;
	int32_t m_width;
	int32_t m_height;

	void traverse(int32_t index, const std::function< void(int32_t) >& fn) const;
};

	}
}