#ifndef traktor_world_PostProcessStepBlur_H
#define traktor_world_PostProcessStepBlur_H

#include "Core/Containers/AlignedVector.h"
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

/*! \brief Post blur step.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepBlur : public PostProcessStep
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
			render::handle_t param;
			render::handle_t source;
			uint32_t index;
		};

		InstanceBlur(
			const resource::Proxy< render::Shader >& shader,
			const std::vector< Source >& sources,
			const Vector4& direction,
			const AlignedVector< Vector4 >& gaussianOffsetWeights
		);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		resource::Proxy< render::Shader > m_shader;
		std::vector< Source > m_sources;
		Vector4 m_direction;
		AlignedVector< Vector4 > m_gaussianOffsetWeights;
		render::handle_t m_handleGaussianOffsetWeights;
		render::handle_t m_handleDirection;
		render::handle_t m_handleViewFar;
		render::handle_t m_handleNoiseOffset;
	};

	PostProcessStepBlur();

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const;

	virtual void serialize(ISerializer& s);

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

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

	resource::Id< render::Shader > m_shader;
	std::vector< Source > m_sources;
	Vector4 m_direction;
	int32_t m_taps;
	BlurType m_blurType;
};

	}
}

#endif	// traktor_world_PostProcessStepBlur_H
