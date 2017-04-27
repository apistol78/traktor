/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ImageProcessStepSsao_H
#define traktor_render_ImageProcessStepSsao_H

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

/*! \brief Post SSAO step.
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
		uint32_t index;			/*!< Render target index. */

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
			uint32_t index;
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

		virtual void destroy() T_OVERRIDE T_FINAL;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) T_OVERRIDE T_FINAL;

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
		handle_t m_handleRandomNormals;
		handle_t m_handleRandomRotations;
		handle_t m_handleMagicCoeffs;
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< Shader >& getShader() const { return m_shader; }

	const std::vector< Source >& getSources() const { return m_sources; }

private:
	resource::Id< Shader > m_shader;
	std::vector< Source > m_sources;
};

	}
}

#endif	// traktor_render_ImageProcessStepSsao_H
