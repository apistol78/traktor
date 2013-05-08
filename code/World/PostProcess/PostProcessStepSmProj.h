#ifndef traktor_world_PostProcessStepSmProj_H
#define traktor_world_PostProcessStepSmProj_H

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
class ISimpleTexture;

	}

	namespace world
	{

/*! \brief Shadow mask projection.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepSmProj : public PostProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceSmProj : public Instance
	{
	public:
		InstanceSmProj(
			const PostProcessStepSmProj* step,
			Ref< render::ISimpleTexture > shadowMapDiscRotation[2],
			const resource::Proxy< render::Shader >& shader
		);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		Ref< const PostProcessStepSmProj > m_step;
		Ref< render::ISimpleTexture > m_shadowMapDiscRotation[2];
		resource::Proxy< render::Shader > m_shader;
		uint32_t m_frame;
		render::handle_t m_handleInputColor;
		render::handle_t m_handleInputDepth;
		render::handle_t m_handleShadowMap;
		render::handle_t m_handleShadowMapDiscRotation;
		render::handle_t m_handleShadowMapSizeAndBias;
		render::handle_t m_handleShadowMapPoissonTaps;
		render::handle_t m_handleDepth;
		render::handle_t m_handleMagicCoeffs;
		render::handle_t m_handleViewEdgeTopLeft;
		render::handle_t m_handleViewEdgeTopRight;
		render::handle_t m_handleViewEdgeBottomLeft;
		render::handle_t m_handleViewEdgeBottomRight;
		render::handle_t m_handleViewToLight;
		render::handle_t m_handleLastSlice;
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
};

	}
}

#endif	// traktor_world_PostProcessStepSmProj_H
