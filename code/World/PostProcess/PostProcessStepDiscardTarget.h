#ifndef traktor_world_PostProcessStepDiscardTarget_H
#define traktor_world_PostProcessStepDiscardTarget_H

#include "Render/Types.h"
#include "Resource/Proxy.h"
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

/*! \brief Set render target.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepDiscardTarget : public PostProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceDiscardTarget : public Instance
	{
	public:
		InstanceDiscardTarget(render::handle_t target);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		render::handle_t m_target;
	};

	PostProcessStepDiscardTarget();

	PostProcessStepDiscardTarget(const std::wstring& target);

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const;

	virtual void serialize(ISerializer& s);

	const std::wstring& getTarget() const { return m_target; }

private:
	std::wstring m_target;
};

	}
}

#endif	// traktor_world_PostProcessStepDiscardTarget_H
