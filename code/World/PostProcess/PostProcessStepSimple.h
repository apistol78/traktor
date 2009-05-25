#ifndef traktor_world_PostProcessStepSimple_H
#define traktor_world_PostProcessStepSimple_H

#include <vector>
#include "World/PostProcess/PostProcessStep.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace world
	{

/*! \brief Generic simple step.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepSimple : public PostProcessStep
{
	T_RTTI_CLASS(PostProcessStepSimple)

public:
	struct Source
	{
		std::wstring param;	/*!< Shader parameter name. */
		uint32_t source;	/*!< Render target set source. */
		uint32_t index;		/*!< Render target index. */

		Source();

		bool serialize(Serializer& s);
	};

	virtual bool create(PostProcess* postProcess, render::RenderSystem* renderSystem);

	virtual void destroy(PostProcess* postProcess);

	virtual void render(
		PostProcess* postProcess,
		const WorldRenderView& worldRenderView,
		render::RenderView* renderView,
		render::ScreenRenderer* screenRenderer,
		float deltaTime
	);

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Proxy< render::Shader > m_shader;
	std::vector< Source > m_sources;
	float m_time;
};

	}
}

#endif	// traktor_world_PostProcessStepSimple_H
