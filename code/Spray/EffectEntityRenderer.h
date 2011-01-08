#ifndef traktor_spray_EffectEntityRenderer_H
#define traktor_spray_EffectEntityRenderer_H

#include "Render/Types.h"
#include "World/Entity/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;

	}

	namespace spray
	{

class PointRenderer;
class EffectEntity;

/*! \brief Effect entity renderer.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	EffectEntityRenderer(render::IRenderSystem* renderSystem);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		world::Entity* entity
	);

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

private:
	Ref< PointRenderer > m_pointRenderer;
	render::handle_t m_defaultTechnique;
};

	}
}

#endif	// traktor_spray_EffectEntityRenderer_H
