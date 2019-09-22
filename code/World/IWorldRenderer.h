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

class Color4f;

	namespace render
	{

class ImageProcess;
class ImageProcessSettings;
class IRenderSystem;
class IRenderView;
class ITexture;
class RenderTargetSet;

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
	int32_t width;
	int32_t height;
	uint32_t multiSample;
	uint32_t frameCount;
	float gamma;
	bool allTargetsPersistent;
	render::RenderTargetSet* sharedDepthStencil;	/*!< Share depth with this render target for all intermediate RTs; useful when rendering to other RT than primary. */

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
	,	width(0)
	,	height(0)
	,	multiSample(0)
	,	frameCount(0)
	,	gamma(2.2f)
	,	allTargetsPersistent(false)
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

	/*! Attach entity to build. 
	 *
	 * \param entity Entity to attach to build.
	 */
	virtual void attach(Entity* entity) = 0;

	/*! Build "render contexts".
	 *
	 * \param worldRenderView World render view.
	 * \param frame Multi threaded context frame.
	 * \param entity Root entity.
	 */
	virtual void build(WorldRenderView& worldRenderView, int32_t frame) = 0;

	//@}

	/*! \name Render steps. */
	//@{

	/*! Begin render "render contexts".
	 *
	 * \param frame Multi threaded context frame.
	 * \param clearColor Clear visual target color.
	 * \return True if rendering begun.
	 */
	virtual bool beginRender(render::IRenderView* renderView, int32_t frame, const Color4f& clearColor) = 0;

	/*! Render "render contexts".
	 *
	 * \param frame Multi threaded context frame.
	 */
	virtual void render(render::IRenderView* renderView, int32_t frame) = 0;

	/*! End render "render contexts".
	 *
	 * \param frame Multi threaded context frame.
	 * \param deltaTime Last frame delta time.
	 */
	virtual void endRender(render::IRenderView* renderView, int32_t frame, float deltaTime) = 0;

	//@}

	/*! \name Target accessor. */
	//@{

	virtual render::ImageProcess* getVisualImageProcess() = 0;

	virtual void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const = 0;

	//@}
};

	}
}

