#ifndef traktor_world_IWorldRenderer_H
#define traktor_world_IWorldRenderer_H

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

/*! \brief World renderer creation description.
 * \ingroup World
 */
struct WorldCreateDesc
{
	const WorldRenderSettings* worldRenderSettings;
	WorldEntityRenderers* entityRenderers;
	Quality motionBlurQuality;
	Quality shadowsQuality;
	Quality ambientOcclusionQuality;
	Quality antiAliasQuality;
	Quality imageProcessQuality;
	int32_t width;
	int32_t height;
	uint32_t multiSample;
	uint32_t superSample;
	uint32_t frameCount;
	float gamma;
	bool allTargetsPersistent;

	WorldCreateDesc()
	:	worldRenderSettings(0)
	,	entityRenderers(0)
	,	motionBlurQuality(QuDisabled)
	,	shadowsQuality(QuDisabled)
	,	ambientOcclusionQuality(QuDisabled)
	,	antiAliasQuality(QuDisabled)
	,	imageProcessQuality(QuDisabled)
	,	width(0)
	,	height(0)
	,	multiSample(0)
	,	superSample(0)
	,	frameCount(0)
	,	gamma(2.2f)
	,	allTargetsPersistent(false)
	{
	}
};

/*! \brief World renderer.
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
	/*! \brief Create world renderer. */
	virtual bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::IRenderView* renderView,
		const WorldCreateDesc& desc
	) = 0;

	/*! \brief Destroy world renderer. */
	virtual void destroy() = 0;

	/*! \name Build steps. */
	//@{

	/*! \brief Begin build "render contexts".
	 *
	 * \return True if building begun.
	 */
	virtual bool beginBuild() = 0;

	/*! \brief Build "render contexts".
	 *
	 * \param entity Root entity.
	 */
	virtual void build(Entity* entity) = 0;

	/*! \brief End build "render contexts".
	 *
	 * \param worldRenderView World render view.
	 * \param frame Multi threaded context frame.
	 */
	virtual void endBuild(WorldRenderView& worldRenderView, int frame) = 0;

	//@}

	/*! \name Render steps. */
	//@{

	/*! \brief Begin render "render contexts".
	 *
	 * \param frame Multi threaded context frame.
	 * \param eye Stereoscopic eye.
	 * \param clearColor Clear visual target color.
	 * \return True if rendering begun.
	 */
	virtual bool beginRender(int frame, render::EyeType eye, const Color4f& clearColor) = 0;

	/*! \brief Render "render contexts".
	 *
	 * \param frame Multi threaded context frame.
	 * \param eye Stereoscopic eye.
	 */
	virtual void render(int frame, render::EyeType eye) = 0;

	/*! \brief End render "render contexts".
	 *
	 * \param frame Multi threaded context frame.
	 * \param eye Stereoscopic eye.
	 * \param deltaTime Last frame delta time.
	 */
	virtual void endRender(int frame, render::EyeType eye, float deltaTime) = 0;

	//@}

	/*! \name Target accessor. */
	//@{

	virtual render::ImageProcess* getVisualImageProcess() = 0;

	virtual void getDebugTargets(std::vector< render::DebugTarget >& outTargets) const = 0;

	//@}
};

	}
}

#endif	// traktor_world_IWorldRenderer_H
