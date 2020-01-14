#pragma once

#include <vector>
#include "Render/Types.h"
#include "Resource/Id.h"
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

/*! \brief
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepGodRay : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	struct Source
	{
		std::wstring param;		/*!< Shader parameter name. */
		std::wstring source;	/*!< Render target set source. */

		Source();

		void serialize(ISerializer& s);
	};

	class InstanceGodRay : public Instance
	{
	public:
		struct Source
		{
			handle_t param;
			handle_t source;
		};

		InstanceGodRay(const ImageProcessStepGodRay* step, const resource::Proxy< Shader >& shader, const std::vector< Source >& sources);

		virtual void destroy() override final;

		virtual void render(
			ImageProcess* imageProcess,
			RenderContext* renderContext,
			ProgramParameters* sharedParams,
			const RenderParams& params
		) override final;

	private:
		Ref< const ImageProcessStepGodRay > m_step;
		resource::Proxy< Shader > m_shader;
		std::vector< Source > m_sources;
		float m_time;
		handle_t m_handleTime;
		handle_t m_handleDeltaTime;
		handle_t m_handleAlpha;
		handle_t m_handleScreenLightPosition;
	};

	ImageProcessStepGodRay();

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< Shader >& getShader() const { return m_shader; }

	const std::vector< Source >& getSources() const { return m_sources; }

private:
	resource::Id< Shader > m_shader;
	std::vector< Source > m_sources;
	float m_lightDistance;
};

	}
}

