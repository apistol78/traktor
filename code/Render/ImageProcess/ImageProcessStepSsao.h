#pragma once

#include <vector>
#include "Core/Math/Random.h"
#include "Core/Math/Vector4.h"
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

/*! Post SSAO step.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepSsao : public ImageProcessStep
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

	class InstanceSsao : public Instance
	{
	public:
		struct Source
		{
			handle_t param;
			handle_t source;
		};

		InstanceSsao(
			const ImageProcessStepSsao* step,
			const std::vector< Source >& sources,
			const Vector4 offsets[64],
			const Vector4 directions[8],
			const resource::Proxy< Shader >& shader,
			ISimpleTexture* randomNormals,
			ISimpleTexture* randomRotations
		);

		virtual void destroy() override final;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) override final;

	private:
		Ref< const ImageProcessStepSsao > m_step;
		std::vector< Source > m_sources;
		Vector4 m_offsets[64];
		Vector4 m_directions[8];
		resource::Proxy< Shader > m_shader;
		Ref< ISimpleTexture > m_randomNormals;
		Ref< ISimpleTexture > m_randomRotations;
		handle_t m_handleViewEdgeTopLeft;
		handle_t m_handleViewEdgeTopRight;
		handle_t m_handleViewEdgeBottomLeft;
		handle_t m_handleViewEdgeBottomRight;
		handle_t m_handleProjection;
		handle_t m_handleOffsets;
		handle_t m_handleDirections;
		handle_t m_handleRandom;
		handle_t m_handleRandomNormals;
		handle_t m_handleRandomRotations;
		handle_t m_handleMagicCoeffs;
		Random m_random;
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
};

	}
}

