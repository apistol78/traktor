#ifndef traktor_world_PostProcessStepLensDirt_H
#define traktor_world_PostProcessStepLensDirt_H

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
class VertexBuffer;

	}

	namespace world
	{

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepLensDirt : public PostProcessStep
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

	class InstanceLensDirt : public Instance
	{
	public:
		enum { InstanceCount = 64 };

		struct Source
		{
			render::handle_t param;
			render::handle_t source;
			uint32_t index;
		};

		InstanceLensDirt(const PostProcessStepLensDirt* step);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		friend class PostProcessStepLensDirt;

		Ref< const PostProcessStepLensDirt > m_step;
		resource::Proxy< render::Shader > m_shader;
		std::vector< Source > m_sources;
		Ref< render::VertexBuffer > m_vertexBuffer;
		render::Primitives m_primitives;
		render::handle_t m_handleInstances;
		Vector4 m_instances[InstanceCount];
	};

	PostProcessStepLensDirt();

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

#endif	// traktor_world_PostProcessStepLensDirt_H
