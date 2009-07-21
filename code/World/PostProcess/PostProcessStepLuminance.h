#ifndef traktor_world_PostProcessStepLuminance_H
#define traktor_world_PostProcessStepLuminance_H

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

	}

	namespace world
	{

/*! \brief Luminance step.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepLuminance : public PostProcessStep
{
	T_RTTI_CLASS(PostProcessStepLuminance)

public:
	virtual bool create(PostProcess* postProcess, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem);

	virtual void destroy(PostProcess* postProcess);

	virtual void render(
		PostProcess* postProcess,
		const WorldRenderView& worldRenderView,
		render::IRenderView* renderView,
		render::ScreenRenderer* screenRenderer,
		float deltaTime
	);

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Proxy< render::Shader > m_shader;
	uint32_t m_source;
	Vector4 m_sampleOffsets[16];
};

	}
}

#endif	// traktor_world_PostProcessStepLuminance_H
