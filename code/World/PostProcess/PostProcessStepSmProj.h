#ifndef traktor_world_PostProcessStepSmProj_H
#define traktor_world_PostProcessStepSmProj_H

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
class ISimpleTexture;

	}

	namespace world
	{

/*! \brief Shadow mask projection.
 * \ingroup World
 */
class T_DLLCLASS PostProcessStepSmProj : public PostProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceSmProj : public Instance
	{
	public:
		InstanceSmProj(
			const PostProcessStepSmProj* step,
			Ref< render::ISimpleTexture > shadowMapDiscRotation[2]
		);

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
		Ref< const PostProcessStepSmProj > m_step;
		Ref< render::ISimpleTexture > m_shadowMapDiscRotation[2];
		uint32_t m_frame;
	};

	virtual Ref< Instance > create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual bool serialize(ISerializer& s);

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	mutable resource::Proxy< render::Shader > m_shader;
};

	}
}

#endif	// traktor_world_PostProcessStepSmProj_H
