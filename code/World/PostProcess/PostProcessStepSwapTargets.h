#ifndef traktor_world_PostProcessStepSwapTargets_H
#define traktor_world_PostProcessStepSwapTargets_H

#include "Render/Types.h"
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

/*! \brief Swap render targets.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepSwapTargets : public PostProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceSwapTargets : public Instance
	{
	public:
		InstanceSwapTargets(render::handle_t destination, render::handle_t source);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		render::handle_t m_destination;
		render::handle_t m_source;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const;

	virtual void serialize(ISerializer& s);

	const std::wstring& getDestination() const { return m_destination; }

	const std::wstring& getSource() const { return m_source; }

private:
	std::wstring m_destination;
	std::wstring m_source;
};

	}
}

#endif	// traktor_world_PostProcessStepSwapTargets_H
