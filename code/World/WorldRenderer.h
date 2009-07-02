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
	namespace render
	{

class RenderSystem;
class RenderView;
class RenderTargetSet;
class SimpleTexture;

	}

	namespace world
	{

//class EntityRenderer;
class Entity;
class WorldContext;
class WorldRenderView;
class WorldEntityRenderers;
class PostProcess;

struct WorldViewPort
{
	int width;
	int height;
	float aspect;
	float fov;

	WorldViewPort()
	:	width(0)
	,	height(0)
	,	aspect(1.0f)
	,	fov(deg2rad(80.0f))
	{
	}
};

/*! \brief Calculate light-view projection used by shadow mapping.
 * \ingroup World
 *
 * We provide a default implementation if none is given.
 * Default implementation tries to maximize view utilization
 * by projecting view frustum.
 */
class T_DLLCLASS LightViewProjection : public Object
{
public:
	enum ShadowPass
	{
		SpOccluders = 1,	//< Occluders pass; occluder geometry is flatten onto shadow map.
		SpSelf = 2			//< Self shadowing pass.
	};

	virtual void calculateLightViewProjection(
		const WorldRenderSettings& settings,
		const Matrix44& viewInverse,
		const Vector4& lightPosition,
		const Vector4& lightDirection,
		const Frustum& viewFrustum,
		Matrix44& outLightView,
		Matrix44& outLightProjectionOccluders,
		Matrix44& outLightProjectionSelfShadow,
		Frustum& outShadowFrustumOccluders,
		Frustum& outShadowFrustumSelfShadow,
		uint32_t& outShadowPasses
	) const = 0;
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
		const WorldRenderSettings& settings,
		WorldEntityRenderers* entityRenderers,
		render::RenderSystem* renderSystem,
		render::RenderView* renderView,
		const WorldViewPort& worldViewPort,
		int multiSample,
		int frameCount
	);

	void destroy();

	/*! \brief Create a world render view.
	 *
	 * \param outRenderView Initialized world render view.
	 */
	void createRenderView(WorldRenderView& outRenderView) const;

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

	inline void setLightViewProjection(LightViewProjection* lightViewProjection) {
		m_lightViewProjection = lightViewProjection;
	}

	inline LightViewProjection* getLightViewProjection() const {
		return m_lightViewProjection;
	}

	inline static render::handle_t getTechniqueDefault() {
		return ms_techniqueDefault;
	}

	inline static render::handle_t getTechniqueDepth() {
		return ms_techniqueDepth;
	}

	inline static render::handle_t getTechniqueShadowMapOccluders() {
		return ms_techniqueShadowMapOccluders;
	}

	inline static render::handle_t getTechniqueShadowMapSelfShadow() {
		return ms_techniqueShadowMapSelfShadow;
	}

	inline render::RenderTargetSet* getDepthTargetSet() const {
		return m_depthTargetSet;
	}

	inline render::RenderTargetSet* getShadowTargetSet() const {
		return m_shadowTargetSet;
	}

private:
	struct Frame
	{
		RefArray< WorldContext > occluders;
		RefArray< WorldContext > selfShadow;
		RefArray< WorldContext > visual;
		Ref< WorldContext > depth;
		Frustum viewFrustum;
		Matrix44 projection;
		float deltaTime;
		bool haveDepth;
		bool haveShadows;
	};

	static render::handle_t ms_techniqueDefault;
	static render::handle_t ms_techniqueDepth;
	static render::handle_t ms_techniqueShadowMapOccluders;
	static render::handle_t ms_techniqueShadowMapSelfShadow;

	Ref< LightViewProjection > m_lightViewProjection;
	WorldRenderSettings m_settings;
	Ref< render::RenderView > m_renderView;
	Ref< render::RenderTargetSet > m_depthTargetSet;
	Ref< render::RenderTargetSet > m_shadowTargetSet;
	Ref< render::SimpleTexture > m_shadowDiscRotation[2];
	WorldViewPort m_worldViewPort;
	AlignedVector< float > m_splitPositions;
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
