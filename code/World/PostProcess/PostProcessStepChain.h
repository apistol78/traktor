#ifndef traktor_world_PostProcessStepChain_H
#define traktor_world_PostProcessStepChain_H

#include "Core/RefArray.h"
#include "World/PostProcess/PostProcessStep.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS;

public:
	class InstanceChain : public Instance
	{
	public:
		InstanceChain(const RefArray< Instance >& instances);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		RefArray< Instance > m_instances;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const;

	virtual void serialize(ISerializer& s);

	inline const RefArray< PostProcessStep >& getSteps() const { return m_steps; }

private:
	RefArray< PostProcessStep > m_steps;
};

	}
}

#endif	// traktor_world_PostProcessStepChain_H
