#ifndef traktor_world_PostProcessStepLuminance_H
#define traktor_world_PostProcessStepLuminance_H

#include "Core/Math/Vector4.h"
#include "Render/Types.h"
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

/*! \brief Luminance step.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepLuminance : public PostProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceLuminance : public Instance
	{
	public:
		InstanceLuminance(
			const PostProcessStepLuminance* step,
			render::handle_t source,
			const Vector4 sampleOffsets[16]
		);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const RenderParams& params
		);

	private:
		Ref< const PostProcessStepLuminance > m_step;
		render::handle_t m_source;
		Vector4 m_sampleOffsets[16];
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const;

	virtual bool serialize(ISerializer& s);

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	mutable resource::Proxy< render::Shader > m_shader;
	std::wstring m_source;
};

	}
}

#endif	// traktor_world_PostProcessStepLuminance_H
