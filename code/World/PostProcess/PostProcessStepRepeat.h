#ifndef traktor_world_PostProcessStepRepeat_H
#define traktor_world_PostProcessStepRepeat_H

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

/*! \brief Repeat step any number of times.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepRepeat : public PostProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceRepeat : public Instance
	{
	public:
		InstanceRepeat(uint32_t count, Instance* instance);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		uint32_t m_count;
		Ref< Instance > m_instance;
	};

	PostProcessStepRepeat();

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const;

	virtual bool serialize(ISerializer& s);

	const Ref< PostProcessStep >& getStep() const { return m_step; }

private:
	uint32_t m_count;
	Ref< PostProcessStep > m_step;
};

	}
}

#endif	// traktor_world_PostProcessStepRepeat_H
