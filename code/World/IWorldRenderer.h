#pragma once

#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Math/Const.h"
#include "Render/Types.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
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
class RenderGraph;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class Entity;
class WorldEntityRenderers;
class WorldRenderSettings;
class WorldRenderView;

/*! World renderer creation description.
 * \ingroup World
 */
struct WorldCreateDesc
{
	const WorldRenderSettings* worldRenderSettings;
	WorldEntityRenderers* entityRenderers;
	Quality toneMapQuality;
	Quality motionBlurQuality;
	Quality shadowsQuality;
	Quality reflectionsQuality;
	Quality ambientOcclusionQuality;
	Quality antiAliasQuality;
	Quality imageProcessQuality;
	uint32_t multiSample;
	uint32_t frameCount;
	float gamma;
	render::IRenderTargetSet* sharedDepthStencil;	/*!< Share depth with this render target for all intermediate RTs; useful when rendering to other RT than primary. */

	WorldCreateDesc()
	:	worldRenderSettings(0)
	,	entityRenderers(0)
	,	toneMapQuality(QuMedium)
	,	motionBlurQuality(QuDisabled)
	,	shadowsQuality(QuDisabled)
	,	reflectionsQuality(QuDisabled)
	,	ambientOcclusionQuality(QuDisabled)
	,	antiAliasQuality(QuDisabled)
	,	imageProcessQuality(QuDisabled)
	,	multiSample(0)
	,	frameCount(0)
	,	gamma(2.2f)
	,	sharedDepthStencil(nullptr)
	{
	}
};

/*! World renderer.
 * \ingroup World
 *
 * The world renderer is a high level renderer which render
 * entities through specialized entity renderer.
 * In order to maximize throughput the world renderer is designed with
 * threading and multiple cores in mind as the rendering is split
 * into two parts, one culling and collecting part and one actual rendering
 * part.
 */
class T_DLLCLASS IWorldRenderer : public Object
{
	T_RTTI_CLASS;

public:
	/*! Create world renderer. */
	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const WorldCreateDesc& desc
	) = 0;

	/*! Destroy world renderer. */
	virtual void destroy() = 0;

	/*! \name Build steps. */
	//@{

	/*! Setup render passes.
	 *
	 * \param worldRenderView World render view.
	 * \param rootEntity Root entity.
	 * \param renderGraph Setup into render graph.
	 * \param outputTargetSetId ID of output target set.
	 */
	virtual void setup(
		const WorldRenderView& worldRenderView,
		const Entity* rootEntity,
		render::RenderGraph& renderGraph,
		render::handle_t outputTargetSetId
	) = 0;

	//@}
};

	}
}

