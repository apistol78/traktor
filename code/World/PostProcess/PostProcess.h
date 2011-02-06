#ifndef traktor_world_PostProcess_H
#define traktor_world_PostProcess_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Types.h"
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
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;
class Shader;
class RenderTargetSet;
class ScreenRenderer;

	}

	namespace world
	{

class PostProcessSettings;

/*! \brief Frame buffer post processing system.
 * \ingroup World
 *
 * Predefined targets:
 * 0  - Frame buffer, write only.
 * -1 - Source color buffer, read only.
 * -2 - Source depth buffer, read only.
 * -4 - Source shadow mask, read only.
 */
class T_DLLCLASS PostProcess : public Object
{
	T_RTTI_CLASS;

public:
	enum PredefinedTargets
	{
		PdtFrame = 0,
		PdtSourceColor = -1,
		PdtSourceDepth = -2,
		PdtSourceShadowMask = -4
	};

	PostProcess();

	bool create(
		const PostProcessSettings* settings,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	);

	void destroy();

	bool render(
		render::IRenderView* renderView,
		render::RenderTargetSet* colorBuffer,
		render::RenderTargetSet* depthBuffer,
		render::RenderTargetSet* shadowMask,
		const PostProcessStep::Instance::RenderParams& params
	);

	void setTarget(render::IRenderView* renderView, int32_t id);

	Ref< render::RenderTargetSet >& getTargetRef(int32_t id);

	void setParameter(render::handle_t handle, bool value);

	void setParameter(render::handle_t handle, float value);

	void prepareShader(render::Shader* shader) const;

	bool requireHighRange() const;

private:
	Ref< render::ScreenRenderer > m_screenRenderer;
	SmallMap< int32_t, Ref< render::RenderTargetSet > > m_targets;
	RefArray< PostProcessStep::Instance > m_instances;
	SmallMap< render::handle_t, bool > m_booleanParameters;
	SmallMap< render::handle_t, float > m_scalarParameters;
	Ref< render::RenderTargetSet > m_currentTarget;
	bool m_requireHighRange;
	Semaphore m_lock;
};

	}
}

#endif	// traktor_world_PostProcess_H
