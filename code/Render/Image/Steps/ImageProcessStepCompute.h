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

/*! Generic compute step.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepCompute : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	struct Source
	{
		std::wstring param;		/*!< Shader parameter name. */
		std::wstring source;	/*!< Render target set source. */

		void serialize(ISerializer& s);
	};

	class InstanceCompute : public Instance
	{
	public:
		struct Source
		{
			handle_t param;
			handle_t source;
		};

		InstanceCompute(const ImageProcessStepCompute* step, const resource::Proxy< Shader >& shader, const std::vector< Source >& sources);

		virtual void destroy() override final;

		virtual void build(
			ImageProcess* imageProcess,
			RenderContext* renderContext,
			ProgramParameters* sharedParams,
			const RenderParams& params
		) override final;

	private:
		Ref< const ImageProcessStepCompute > m_step;
		resource::Proxy< Shader > m_shader;
		std::vector< Source > m_sources;
		float m_time;
		handle_t m_handleTime;
		handle_t m_handleDeltaTime;
	};

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
	int32_t m_workSize[3];
};

	}
}
