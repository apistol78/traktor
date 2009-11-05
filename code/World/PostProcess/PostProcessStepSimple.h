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
		int32_t source;		/*!< Render target set source. */
		uint32_t index;		/*!< Render target index. */

		Source();

		bool serialize(Serializer& s);
	};

	class InstanceSimple : public Instance
	{
	public:
		InstanceSimple(const PostProcessStepSimple* step);

		virtual void destroy();

		virtual void render(
			PostProcess* postProcess,
			render::IRenderView* renderView,
			render::ScreenRenderer* screenRenderer,
			const Frustum& viewFrustum,
			const Matrix44& projection,
			float shadowMapBias,
			float deltaTime
		);

	private:
		Ref< const PostProcessStepSimple > m_step;
		float m_time;
	};

	virtual Instance* create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	mutable resource::Proxy< render::Shader > m_shader;
	std::vector< Source > m_sources;
};

	}
}

#endif	// traktor_world_PostProcessStepSimple_H
