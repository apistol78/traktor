#ifndef traktor_world_WorldRenderer_H
#define traktor_world_WorldRenderer_H

//#include <map>
#include <hash_map>
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

class EntityRenderer;
class Entity;
class WorldContext;
class WorldRenderView;
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

// Don't share shadow map across slices on Xbox360 as we cannot render
// multiple times to same render target on Xbox360.
#if !defined(_XBOX)
#	define T_SHARE_SLICE_SHADOWMAP 1
#else
#	define T_SHARE_SLICE_SHADOWMAP 0
#endif

typedef stdext::hash_map< const Type*, EntityRenderer* > entity_renderer_map_t;
//typedef std::map< const Type*, EntityRenderer* > entity_renderer_map_t;

/*! \brief Calculate light-view projection used by shadow mapping.
 * \ingroup World
 *
 * We provide a default implementation if noone is given.
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

/*! \brief World renderer.
 * \ingroup World
 *
 * The world renderer is a high level renderer which renders
 * entities through specialized entity renderers.
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
		render::RenderSystem* renderSystem,
		render::RenderView* renderView,
		const WorldViewPort& worldViewPort,
		int multiSample,
		int frameCount
	);

	void destroy();

	/*! \brief Add entity renderer.
	 *
	 * \param entityRenderer Specialized entity renderer.
	 */
	void addEntityRenderer(EntityRenderer* entityRenderer);

	/*! \brief Remove entity renderer.
	 *
	 * \param entityRenderer Previously added entity renderer.
	 */
	void removeEntityRenderer(EntityRenderer* entityRenderer);

	/*! \brief Find entity renderer.
	 *
	 * Find entity renderer which supports given
	 * entity type.
	 *
	 * \param entityType Type of entity to render.
	 * \return Specialized entity renderer, null if noone was found.
	 */
	EntityRenderer* findEntityRenderer(const Type* entityType);

	/*! \brief Get array of all entity renderers.
	 *
	 * \return Array of entity renderers.
	 */
	const RefArray< EntityRenderer >& getEntityRenderers() const;

	/*! \brief Create a world render view.
	 *
	 * \param outRenderView Initialized world render view.
	 */
	void createRenderView(WorldRenderView& outRenderView) const;

	/*! \name Render steps. */
	//@{

	/*! \brief Render world from world view.
	 *
	 * \param worldRenderView World render view.
	 * \param deltaTime Current delta time.
	 * \param entity Root entity.
	 * \param frame Multi threaded context frame.
	 */
	void render(WorldRenderView& worldRenderView, float deltaTime, Entity* entity, int frame);

	/*! \brief Flush render contexts.
	 *
	 * \param postProcess Post process settings; null if no post processing is to be used.
	 * \param frame Multi threaded context frame.
	 */
	void flush(PostProcess* postProcess, int frame);

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

	inline render::RenderTargetSet* getRenderTargetSet() const {
		return m_renderTargetSet;
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
	};

	static render::handle_t ms_techniqueDefault;
	static render::handle_t ms_techniqueDepth;
	static render::handle_t ms_techniqueShadowMapOccluders;
	static render::handle_t ms_techniqueShadowMapSelfShadow;

	Ref< LightViewProjection > m_lightViewProjection;
	WorldRenderSettings m_settings;
	Ref< render::RenderView > m_renderView;
	Ref< render::RenderTargetSet > m_renderTargetSet;
	Ref< render::RenderTargetSet > m_shadowTargetSet;
	Ref< render::SimpleTexture > m_shadowDiscRotation[2];
	WorldViewPort m_worldViewPort;
	RefArray< EntityRenderer > m_entityRenderers;
	entity_renderer_map_t m_entityRendererMap;
	AlignedVector< float > m_splitPositions;
	AlignedVector< Frame > m_frames;
	float m_time;
	uint32_t m_count;
};

	}
}

#endif	// traktor_world_WorldRenderer_H
