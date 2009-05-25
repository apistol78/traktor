#ifndef traktor_world_PostProcessStepSwapTargets_H
#define traktor_world_PostProcessStepSwapTargets_H

#include "World/PostProcess/PostProcessStep.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Swap render targets.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepSwapTargets : public PostProcessStep
{
	T_RTTI_CLASS(PostProcessStepSwapTargets)

public:
	virtual bool create(PostProcess* postProcess, render::RenderSystem* renderSystem);

	virtual void destroy(PostProcess* postProcess);

	virtual void render(
		PostProcess* postProcess,
		const WorldRenderView& worldRenderView,
		render::RenderView* renderView,
		render::ScreenRenderer* screenRenderer,
		float deltaTime
	);

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_destination;
	uint32_t m_source;
};

	}
}

#endif	// traktor_world_PostProcessStepSwapTargets_H
