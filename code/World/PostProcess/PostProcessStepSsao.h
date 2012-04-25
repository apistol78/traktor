#ifndef traktor_world_PostProcessStepSsao_H
#define traktor_world_PostProcessStepSsao_H

#include "World/PostProcess/PostProcessStep.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "Core/Math/Vector4.h"

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
class ISimpleTexture;

	}

	namespace world
	{

/*! \brief Post SSAO step.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepSsao : public PostProcessStep
{
	T_RTTI_CLASS;

public:
	struct Source
	{
		std::wstring param;		/*!< Shader parameter name. */
		std::wstring source;	/*!< Render target set source. */
		uint32_t index;			/*!< Render target index. */

		Source();

		bool serialize(ISerializer& s);
	};

	class InstanceSsao : public Instance
	{
	public:
		struct Source
		{
			render::handle_t param;
			render::handle_t paramSize;
			render::handle_t source;
			uint32_t index;
		};

		InstanceSsao(
			const PostProcessStepSsao* step,
			const std::vector< Source >& sources,
			const Vector4 offsets[32],
			const resource::Proxy< render::Shader >& shader,
			render::ISimpleTexture* randomNormals
		);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		Ref< const PostProcessStepSsao > m_step;
		std::vector< Source > m_sources;
		Vector4 m_offsets[32];
		resource::Proxy< render::Shader > m_shader;
		Ref< render::ISimpleTexture > m_randomNormals;
		render::handle_t m_handleInputColor;
		render::handle_t m_handleInputDepth;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const;

	virtual bool serialize(ISerializer& s);

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< render::Shader > m_shader;
	std::vector< Source > m_sources;
};

	}
}

#endif	// traktor_world_PostProcessStepSsao_H
