#ifndef traktor_world_PostProcessStepRepeat_H
#define traktor_world_PostProcessStepRepeat_H

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

/*! \brief Repeat step any number of times.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepRepeat : public PostProcessStep
{
	T_RTTI_CLASS(PostProcessStepRepeat)

public:
	virtual bool create(PostProcess* postProcess, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	virtual void destroy(PostProcess* postProcess);

	virtual void render(
		PostProcess* postProcess,
		const WorldRenderView& worldRenderView,
		render::IRenderView* renderView,
		render::ScreenRenderer* screenRenderer,
		float deltaTime
	);

	virtual bool serialize(Serializer& s);

	inline const Ref< PostProcessStep >& getStep() const { return m_step; }

private:
	uint32_t m_count;
	Ref< PostProcessStep > m_step;
};

	}
}

#endif	// traktor_world_PostProcessStepRepeat_H
