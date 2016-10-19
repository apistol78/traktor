#ifndef traktor_render_ImageProcessStepSmProj_H
#define traktor_render_ImageProcessStepSmProj_H

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

class Shader;
class ISimpleTexture;

/*! \brief Shadow mask projection.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepSmProj : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceSmProj : public Instance
	{
	public:
		InstanceSmProj(
			const ImageProcessStepSmProj* step,
			Ref< ISimpleTexture > shadowMapDiscRotation[2],
			const resource::Proxy< Shader >& shader
		);

		virtual void destroy() T_OVERRIDE T_FINAL;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) T_OVERRIDE T_FINAL;

	private:
		Ref< const ImageProcessStepSmProj > m_step;
		Ref< ISimpleTexture > m_shadowMapDiscRotation[2];
		resource::Proxy< Shader > m_shader;
		uint32_t m_frame;
		handle_t m_handleInputColor;
		handle_t m_handleInputDepth;
		handle_t m_handleShadowMapColor;
		handle_t m_handleShadowMapDepth;
		handle_t m_handleShadowMapDiscRotation;
		handle_t m_handleShadowMapSizeAndBias;
		handle_t m_handleShadowMapPoissonTaps;
		handle_t m_handleDepth;
		handle_t m_handleMagicCoeffs;
		handle_t m_handleViewEdgeTopLeft;
		handle_t m_handleViewEdgeTopRight;
		handle_t m_handleViewEdgeBottomLeft;
		handle_t m_handleViewEdgeBottomRight;
		handle_t m_handleViewToLight;
		handle_t m_handleLastSlice;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< Shader >& getShader() const { return m_shader; }

private:
	resource::Id< Shader > m_shader;
};

	}
}

#endif	// traktor_render_ImageProcessStepSmProj_H
