#ifndef traktor_world_PostProcessStepTemporal_H
#define traktor_world_PostProcessStepTemporal_H

#include <vector>
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

class Shader;

	}

	namespace world
	{

/*! \brief Step useful for temporal filtering.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepTemporal : public PostProcessStep
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

	class InstanceTemporal : public Instance
	{
	public:
		struct Source
		{
			render::handle_t param;
			render::handle_t source;
			uint32_t index;
		};

		InstanceTemporal(const PostProcessStepTemporal* step, const resource::Proxy< render::Shader >& shader, const std::vector< Source >& sources);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		Ref< const PostProcessStepTemporal > m_step;
		resource::Proxy< render::Shader > m_shader;
		std::vector< Source > m_sources;
		float m_time;
		Matrix44 m_previousView;
		render::handle_t m_handleTime;
		render::handle_t m_handleDeltaTime;
		render::handle_t m_handleViewEdgeTopLeft;
		render::handle_t m_handleViewEdgeTopRight;
		render::handle_t m_handleViewEdgeBottomLeft;
		render::handle_t m_handleViewEdgeBottomRight;
		render::handle_t m_handleProjection;
		render::handle_t m_handleDeltaView;
		render::handle_t m_handleDeltaViewProj;
		render::handle_t m_handleMagicCoeffs;
	};

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

#endif	// traktor_world_PostProcessStepTemporal_H
