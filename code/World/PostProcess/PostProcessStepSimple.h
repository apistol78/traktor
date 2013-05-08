#ifndef traktor_world_PostProcessStepSimple_H
#define traktor_world_PostProcessStepSimple_H

#include <vector>
#include "Render/Types.h"
#include "Resource/Id.h"
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

/*! \brief Generic simple step.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepSimple : public PostProcessStep
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

	class InstanceSimple : public Instance
	{
	public:
		struct Source
		{
			render::handle_t param;
			render::handle_t source;
			uint32_t index;
		};

		InstanceSimple(const PostProcessStepSimple* step, const resource::Proxy< render::Shader >& shader, const std::vector< Source >& sources);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		Ref< const PostProcessStepSimple > m_step;
		resource::Proxy< render::Shader > m_shader;
		std::vector< Source > m_sources;
		float m_time;
		render::handle_t m_handleTime;
		render::handle_t m_handleDeltaTime;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const;

	virtual void serialize(ISerializer& s);

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< render::Shader > m_shader;
	std::vector< Source > m_sources;
};

	}
}

#endif	// traktor_world_PostProcessStepSimple_H
