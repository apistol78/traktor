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
	class InstanceSwapTargets : public Instance
	{
	public:
		InstanceSwapTargets(int32_t destination, int32_t source);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const Frustum& viewFrustum,
			const Matrix44& projection,
			float shadowMapBias,
			float deltaTime
		);

	private:
		int32_t m_destination;
		int32_t m_source;
	};

	virtual Instance* create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual bool serialize(Serializer& s);

private:
	int32_t m_destination;
	int32_t m_source;
};

	}
}

#endif	// traktor_world_PostProcessStepSwapTargets_H
