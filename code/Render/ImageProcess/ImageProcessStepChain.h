#pragma once

#include "Core/RefArray.h"
#include "Render/ImageProcess/ImageProcessStep.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Chain of sub-steps.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepChain : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceChain : public Instance
	{
	public:
		InstanceChain(const RefArray< Instance >& instances);

		virtual void destroy() override final;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) override final;

	private:
		RefArray< Instance > m_instances;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const override final;

	virtual void serialize(ISerializer& s) override final;

	inline const RefArray< ImageProcessStep >& getSteps() const { return m_steps; }

private:
	RefArray< ImageProcessStep > m_steps;
};

	}
}

