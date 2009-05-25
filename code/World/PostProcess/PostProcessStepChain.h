#ifndef traktor_world_PostProcessStepChain_H
#define traktor_world_PostProcessStepChain_H

#include "Core/Heap/Ref.h"
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

/*! \brief Chain of sub-steps.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepChain : public PostProcessStep
{
	T_RTTI_CLASS(PostProcessStepChain)

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

	inline const RefArray< PostProcessStep >& getSteps() const { return m_steps; }

private:
	RefArray< PostProcessStep > m_steps;
};

	}
}

#endif	// traktor_world_PostProcessStepChain_H
