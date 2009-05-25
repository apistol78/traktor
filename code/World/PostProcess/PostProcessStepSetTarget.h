#ifndef traktor_world_PostProcessStepSetTarget_H
#define traktor_world_PostProcessStepSetTarget_H

#include "World/PostProcess/PostProcessStep.h"
#include "Resource/Proxy.h"

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

class Shader;

	}

	namespace world
	{

/*! \brief Set render target.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepSetTarget : public PostProcessStep
{
	T_RTTI_CLASS(PostProcessStepSetTarget)

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
	uint32_t m_target;
};

	}
}

#endif	// traktor_world_PostProcessStepSetTarget_H
