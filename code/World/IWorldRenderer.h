#ifndef traktor_world_IWorldRenderer_H
#define traktor_world_IWorldRenderer_H

#include "Core/Object.h"
#include "Core/Math/Const.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class WorldRenderView;

/*! \brief Perspective view port.
 * \ingroup World
 */
struct WorldViewPerspective
{
	int32_t width;
	int32_t height;
	float aspect;
	float fov;

	WorldViewPerspective()
	:	width(0)
	,	height(0)
	,	aspect(1.0f)
	,	fov(deg2rad(65.0f))
	{
	}
};

/*! \brief Orthogonal view port.
 * \ingroup World
 */
struct WorldViewOrtho
{
	float width;
	float height;

	WorldViewOrtho()
	:	width(0.0f)
	,	height(0.0f)
	{
	}
};

/*! \brief World render flags.
 * \ingroup World
 */
enum WorldRenderFlags
{
	WrfDepthMap = 1,
	WrfShadowMap = 2,
	WrfVisualOpaque = 4,
	WrfVisualAlphaBlend = 8
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
	virtual void destroy() = 0;

	/*! \brief Create a world render view.
	 *
	 * \param worldView World view.
	 * \param outRenderView Initialized world render view.
	 */
	virtual void createRenderView(const WorldViewPerspective& worldView, WorldRenderView& outRenderView) const = 0;

	/*! \brief Create a world render view.
	 *
	 * \param worldView World view.
	 * \param outRenderView Initialized world render view.
	 */
	virtual void createRenderView(const WorldViewOrtho& worldView, WorldRenderView& outRenderView) const = 0;

	/*! \name Render steps. */
	//@{

	/*! \brief Build "render contexts".
	 *
	 * \param worldRenderView World render view.
	 * \param entity Root entity.
	 * \param frame Multi threaded context frame.
	 */
	virtual void build(WorldRenderView& worldRenderView, Entity* entity, int frame) = 0;

	/*! \brief Render "render contexts".
	 *
	 * \param flags Combination of world render flags.
	 * \param frame Multi threaded context frame.
	 */
	virtual void render(uint32_t flags, int frame, render::EyeType eye) = 0;

	//@}
};

	}
}

#endif	// traktor_world_IWorldRenderer_H
