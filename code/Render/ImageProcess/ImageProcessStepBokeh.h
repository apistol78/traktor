#pragma once

#include <vector>
#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
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

class IndexBuffer;
class Shader;
class VertexBuffer;

/*! Post bokeh step.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepBokeh : public ImageProcessStep
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

	class InstanceBokeh : public Instance
	{
	public:
		struct Source
		{
			handle_t param;
			handle_t source;
		};

		InstanceBokeh(
			const resource::Proxy< Shader >& shader,
			VertexBuffer* vertexBuffer,
			IndexBuffer* indexBuffer,
			const std::vector< Source >& sources,
			uint32_t quadCount
		);

		virtual void destroy() override final;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) override final;

	private:
		resource::Proxy< Shader > m_shader;
		Ref< VertexBuffer > m_vertexBuffer;
		Ref< IndexBuffer > m_indexBuffer;
		std::vector< Source > m_sources;
		uint32_t m_quadCount;
		float m_time;
		handle_t m_handleTime;
		handle_t m_handleDeltaTime;
		handle_t m_handleRatio;
	};

	ImageProcessStepBokeh();

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
};

	}
}

