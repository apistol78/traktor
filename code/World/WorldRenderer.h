#ifndef traktor_world_WorldRenderer_H
#define traktor_world_WorldRenderer_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Math/Const.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Frustum.h"
#include "Core/Containers/AlignedVector.h"
#include "World/WorldRenderSettings.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;
class RenderTargetSet;
class ISimpleTexture;

	}

	namespace world
	{

class Entity;
class WorldContext;
class WorldRenderView;
class WorldEntityRenderers;
class PostProcess;

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
	WrfVelocityMap = 4,
	WrfVisualOpaque = 8,
	WrfVisualAlphaBlend = 16
};

/*! \brief World renderer.
 * \ingroup World
 *
 * The world renderer is a high level renderer which renders
 * entities through specialized entity renderer.
 * In order to maximize throughput the world renderer is designed with
 * threading and multiple cores in mind as the rendering is split
 * into two parts, one culling and collecting part and one actual rendering
 * part.
 */
class T_DLLCLASS WorldRenderer : public Object
{
	T_RTTI_CLASS(WorldRenderer)

public:
	WorldRenderer();

	bool create(
		const WorldRenderSettings* settings,
		WorldEntityRenderers* entityRenderers,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::IRenderView* renderView,
		int multiSample,
		int frameCount
	);

	void destroy();

	/*! \brief Create a world render view.
	 *
	 * \param worldView World view.
	 * \param outRenderView Initialized world render view.
	 */
	void createRenderView(const WorldViewPerspective& worldView, WorldRenderView& outRenderView) const;

	/*! \brief Create a world render view.
	 *
	 * \param worldView World view.
	 * \param outRenderView Initialized world render view.
	 */
	void createRenderView(const WorldViewOrtho& worldView, WorldRenderView& outRenderView) const;

	/*! \name Render steps. */
	//@{

	/*! \brief Build "render contexts".
	 *
	 * \param worldRenderView World render view.
	 * \param deltaTime Current delta time.
	 * \param entity Root entity.
	 * \param frame Multi threaded context frame.
	 */
	void build(WorldRenderView& worldRenderView, float deltaTime, Entity* entity, int frame);

	/*! \brief Render "render contexts".
	 *
	 * \param flags Combination of world render flags.
	 * \param frame Multi threaded context frame.
	 */
	void render(uint32_t flags, int frame);

	/*! \brief Flush render contexts.
	 *
	 * \param frame Multi threaded context frame.
	 */
	void flush(int frame);

	//@}

	inline static render::handle_t getTechniqueDefault() {
		return ms_techniqueDefault;
	}

	inline static render::handle_t getTechniqueDepth() {
		return ms_techniqueDepth;
	}

	inline static render::handle_t getTechniqueVelocity() {
		return ms_techniqueVelocity;
	}

	inline static render::handle_t getTechniqueShadow() {
		return ms_techniqueShadow;
	}

	inline render::RenderTargetSet* getDepthTargetSet() const {
		return m_depthTargetSet;
	}

	inline render::RenderTargetSet* getVelocityTargetSet() const {
		return m_velocityTargetSet;
	}

	inline render::RenderTargetSet* getShadowTargetSet() const {
		return m_shadowTargetSet;
	}

	inline render::RenderTargetSet* getShadowMaskTargetSet() const {
		return m_shadowMaskTargetSet;
	}

private:
	struct Frame
	{
		Ref< WorldContext > shadow;
		Ref< WorldContext > visual;
		Ref< WorldContext > velocity;
		Ref< WorldContext > depth;
		Frustum viewFrustum;
		Matrix44 viewToLightSpace;
		Matrix44 projection;
		float deltaTime;
		bool haveDepth;
		bool haveVelocity;
		bool haveShadows;

		Frame()
		:	deltaTime(0.0f)
		,	haveDepth(false)
		,	haveVelocity(false)
		,	haveShadows(false)
		{
		}
	};

	static render::handle_t ms_techniqueDefault;
	static render::handle_t ms_techniqueDepth;
	static render::handle_t ms_techniqueVelocity;
	static render::handle_t ms_techniqueShadow;

	WorldRenderSettings m_settings;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderTargetSet > m_depthTargetSet;
	Ref< render::RenderTargetSet > m_velocityTargetSet;
	Ref< render::RenderTargetSet > m_shadowTargetSet;
	Ref< render::RenderTargetSet > m_shadowMaskTargetSet;
	Ref< render::ISimpleTexture > m_shadowDiscRotation[2];
	Ref< PostProcess > m_shadowMaskProjection;
	AlignedVector< Frame > m_frames;
	float m_time;
	uint32_t m_count;

	/*! \brief Build render contexts.
	 *
	 * \note This method renders world with shadows
	 *       regardless of what is specified in the settings.
	 *
	 * \param worldRenderView World render view.
	 * \param deltaTime Current delta time.
	 * \param entity Root entity.
	 * \param frame Multi threaded context frame.
	 */
	void buildShadows(WorldRenderView& worldRenderView, float deltaTime, Entity* entity, int frame);

	/*! \brief Build render contexts.
	 *
	 * \note This method renders world without shadows
	 *       regardless of what is specified in the settings.
	 *
	 * \param worldRenderView World render view.
	 * \param deltaTime Current delta time.
	 * \param entity Root entity.
	 * \param frame Multi threaded context frame.
	 */
	void buildNoShadows(WorldRenderView& worldRenderView, float deltaTime, Entity* entity, int frame);
};

	}
}

#endif	// traktor_world_WorldRenderer_H
