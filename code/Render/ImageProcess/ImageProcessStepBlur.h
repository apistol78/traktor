/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ImageProcessStepBlur_H
#define traktor_render_ImageProcessStepBlur_H

#include "Core/Containers/AlignedVector.h"
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

/*! \brief Post blur step.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepBlur : public ImageProcessStep
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

	class InstanceBlur : public Instance
	{
	public:
		struct Source
		{
			handle_t param;
			handle_t source;
			uint32_t index;
		};

		InstanceBlur(
			const resource::Proxy< Shader >& shader,
			const std::vector< Source >& sources,
			const Vector4& direction,
			const AlignedVector< Vector4 >& gaussianOffsetWeights
		);

		virtual void destroy() T_OVERRIDE T_FINAL;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) T_OVERRIDE T_FINAL;

	private:
		resource::Proxy< Shader > m_shader;
		std::vector< Source > m_sources;
		Vector4 m_direction;
		AlignedVector< Vector4 > m_gaussianOffsetWeights;
		handle_t m_handleGaussianOffsetWeights;
		handle_t m_handleDirection;
		handle_t m_handleViewFar;
		handle_t m_handleNoiseOffset;
	};

	ImageProcessStepBlur();

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
	enum BlurType
	{
		// 1D separable
		BtGaussian,
		BtSine,
		BtBox,
		// 2D combined
		BtBox2D,
		BtCircle2D
	};

	resource::Id< Shader > m_shader;
	std::vector< Source > m_sources;
	Vector4 m_direction;
	int32_t m_taps;
	BlurType m_blurType;
};

	}
}

#endif	// traktor_render_ImageProcessStepBlur_H
