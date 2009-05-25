#ifndef traktor_world_PostProcessStepSsao_H
#define traktor_world_PostProcessStepSsao_H

#include "World/PostProcess/PostProcessStep.h"
#include "Resource/Proxy.h"
#include "Core/Math/Vector4.h"

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
class SimpleTexture;

	}

	namespace world
	{

/*! \brief Post SSAO step.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepSsao : public PostProcessStep
{
	T_RTTI_CLASS(PostProcessStepSsao)

public:
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
	Vector4 m_offsets[32];
	Ref< render::SimpleTexture > m_randomNormals;
};

	}
}

#endif	// traktor_world_PostProcessStepSsao_H
