#pragma once

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
class ISimpleTexture;

/*! Shadow mask projection.
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

		virtual void destroy() override final;

		virtual void render(
			ImageProcess* imageProcess,
			RenderContext* renderContext,
			ProgramParameters* sharedParams,
			const RenderParams& params
		) override final;

	private:
		Ref< const ImageProcessStepSmProj > m_step;
		Ref< ISimpleTexture > m_shadowMapDiscRotation[2];
		resource::Proxy< Shader > m_shader;
		uint32_t m_frame;
		handle_t m_handleInputColor;
		handle_t m_handleInputDepth;
		handle_t m_handleShadowMap;
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
	) const override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< Shader >& getShader() const { return m_shader; }

private:
	resource::Id< Shader > m_shader;
};

	}
}

