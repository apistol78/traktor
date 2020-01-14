#pragma once

#include "Render/Types.h"
#include "Resource/Proxy.h"
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

class Shader;

/*! Set render target.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepSetTarget : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceSetTarget : public Instance
	{
	public:
		InstanceSetTarget(handle_t target);

		virtual void destroy() override final;

		virtual void render(
			ImageProcess* imageProcess,
			RenderContext* renderContext,
			ProgramParameters* sharedParams,
			const RenderParams& params
		) override final;

	private:
		handle_t m_target;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const override final;

	virtual void serialize(ISerializer& s) override final;

	const std::wstring& getTarget() const { return m_target; }

private:
	std::wstring m_target;
};

	}
}

