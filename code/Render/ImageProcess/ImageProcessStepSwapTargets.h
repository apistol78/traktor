#ifndef traktor_render_ImageProcessStepSwapTargets_H
#define traktor_render_ImageProcessStepSwapTargets_H

#include "Render/Types.h"
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

/*! \brief Swap render targets.
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

		virtual void destroy();

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		handle_t m_destination;
		handle_t m_source;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
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

#endif	// traktor_render_ImageProcessStepSwapTargets_H
