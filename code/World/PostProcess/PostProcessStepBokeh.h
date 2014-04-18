#ifndef traktor_world_PostProcessStepBokeh_H
#define traktor_world_PostProcessStepBokeh_H

#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "Resource/Id.h"
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
	namespace render
	{

class IndexBuffer;
class Shader;
class VertexBuffer;

	}

	namespace world
	{

/*! \brief Post bokeh step.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepBokeh : public PostProcessStep
{
	T_RTTI_CLASS;

public:
	struct Source
	{
		std::wstring param;		/*!< Shader parameter name. */
		std::wstring source;	/*!< Render target set source. */
		uint32_t index;			/*!< Render target index. */

		Source();

		void serialize(ISerializer& s);
	};

	class InstanceBokeh : public Instance
	{
	public:
		struct Source
		{
			render::handle_t param;
			render::handle_t source;
			uint32_t index;
		};

		InstanceBokeh(
			const resource::Proxy< render::Shader >& shader,
			render::VertexBuffer* vertexBuffer,
			render::IndexBuffer* indexBuffer,
			const std::vector< Source >& sources,
			uint32_t quadCount
		);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		resource::Proxy< render::Shader > m_shader;
		Ref< render::VertexBuffer > m_vertexBuffer;
		Ref< render::IndexBuffer > m_indexBuffer;
		std::vector< Source > m_sources;
		uint32_t m_quadCount;
		float m_time;
		render::handle_t m_handleTime;
		render::handle_t m_handleDeltaTime;
		render::handle_t m_handleRatio;
	};

	PostProcessStepBokeh();

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const;

	virtual void serialize(ISerializer& s);

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	const std::vector< Source >& getSources() const { return m_sources; }

private:
	resource::Id< render::Shader > m_shader;
	std::vector< Source > m_sources;
};

	}
}

#endif	// traktor_world_PostProcessStepBokeh_H
