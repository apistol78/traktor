#ifndef traktor_world_PostProcessStepBlur_H
#define traktor_world_PostProcessStepBlur_H

#include "Core/Containers/AlignedVector.h"
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
		std::wstring param;	/*!< Shader parameter name. */
		int32_t source;		/*!< Render target set source. */
		uint32_t index;		/*!< Render target index. */

		Source();

		bool serialize(ISerializer& s);
	};

	class InstanceBlur : public Instance
	{
	public:
		InstanceBlur(
			const PostProcessStepBlur* step,
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
		Ref< const PostProcessStepBlur > m_step;
		AlignedVector< Vector4 > m_gaussianOffsetWeights;
	};

	PostProcessStepBlur();

	virtual Ref< Instance > create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual bool serialize(ISerializer& s);

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	mutable resource::Proxy< render::Shader > m_shader;
	std::vector< Source > m_sources;
	Vector4 m_direction;
	int32_t m_taps;
};

	}
}

#endif	// traktor_world_PostProcessStepBlur_H
