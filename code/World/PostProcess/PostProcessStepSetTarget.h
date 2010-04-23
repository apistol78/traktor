#ifndef traktor_world_PostProcessStepSetTarget_H
#define traktor_world_PostProcessStepSetTarget_H

#include "World/PostProcess/PostProcessStep.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS;

public:
	class InstanceSetTarget : public Instance
	{
	public:
		InstanceSetTarget(int32_t target);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		int32_t m_target;
	};

	virtual Ref< Instance > create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual bool serialize(ISerializer& s);

private:
	int32_t m_target;
};

	}
}

#endif	// traktor_world_PostProcessStepSetTarget_H
