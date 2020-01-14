#pragma once

#include "Render/Types.h"
#include "Render/Image/ImageProcessStep.h"

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

/*! Swap render targets.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepSwapTargets : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceSwapTargets : public Instance
	{
	public:
		InstanceSwapTargets(handle_t destination, handle_t source);

		virtual void destroy() override final;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) override final;

	private:
		handle_t m_destination;
		handle_t m_source;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const override final;

	virtual void serialize(ISerializer& s) override final;

	const std::wstring& getDestination() const { return m_destination; }

	const std::wstring& getSource() const { return m_source; }

private:
	std::wstring m_destination;
	std::wstring m_source;
};

	}
}

