#ifndef traktor_world_PostProcessStep_H
#define traktor_world_PostProcessStep_H

#include "Core/Serialization/Serializable.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"

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

class RenderSystem;
class RenderView;
class ScreenRenderer;

	}

	namespace world
	{

class PostProcess;
class WorldRenderView;

/*! \brief Post processing step.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStep : public Serializable
{
	T_RTTI_CLASS(PostProcessStep)

public:
	virtual bool create(PostProcess* postProcess, resource::IResourceManager* resourceManager, render::RenderSystem* renderSystem) = 0;

	virtual void destroy(PostProcess* postProcess) = 0;

	virtual void render(
		PostProcess* postProcess,
		const WorldRenderView& worldRenderView,
		render::RenderView* renderView,
		render::ScreenRenderer* screenRenderer,
		float deltaTime
	) = 0;
};

	}
}

#endif	// traktor_world_PostProcessStep_H
