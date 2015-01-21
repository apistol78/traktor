#ifndef traktor_world_PostProcess_H
#define traktor_world_PostProcess_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Core/Thread/Semaphore.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/WorldTypes.h"
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
class ITexture;
class Shader;
class RenderTargetSet;
class ScreenRenderer;

	}

	namespace world
	{

class PostProcessSettings;
class PostProcessTargetPool;

/*! \brief Frame buffer post processing system.
 * \ingroup World
 *
 * Predefined targets:
 * "Output" - Frame buffer, write only.
 * "InputColor" - Source color buffer, read only.
 * "InputDepth" - Source depth buffer, read only.
 * "InputShadowMask" - Source shadow mask, read only.
 */
class T_DLLCLASS PostProcess : public Object
{
	T_RTTI_CLASS;

public:
	PostProcess();

	bool create(
		const PostProcessSettings* settings,
		PostProcessTargetPool* targetPool,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height,
		bool allTargetsPersistent
	);

	void destroy();

	bool render(
		render::IRenderView* renderView,
		render::RenderTargetSet* colorBuffer,
		render::RenderTargetSet* depthBuffer,
		render::RenderTargetSet* shadowMask,
		const PostProcessStep::Instance::RenderParams& params
	);

	void defineTarget(const std::wstring& name, render::handle_t id, const render::RenderTargetSetCreateDesc& rtscd, const Color4f& clearColor, bool persistent);

	void setTarget(render::IRenderView* renderView, render::handle_t id);

	render::RenderTargetSet* getTarget(render::handle_t id);

	void swapTargets(render::handle_t id0, render::handle_t id1);

	void discardTarget(render::handle_t id);

	void setCombination(render::handle_t handle, bool value);

	void setFloatParameter(render::handle_t handle, float value);

	void setVectorParameter(render::handle_t handle, const Vector4& value);

	void setTextureParameter(render::handle_t handle, const resource::Proxy< render::ITexture >& value);

	void prepareShader(render::Shader* shader) const;

	bool requireHighRange() const;

	void getDebugTargets(std::vector< DebugTarget >& outTargets) const;

private:
	struct Target
	{
		std::wstring name;
		render::RenderTargetSetCreateDesc rtscd;
		Ref< render::RenderTargetSet > rts;
		float clearColor[4];
		bool shouldClear;
		bool persistent;

		Target()
		:	shouldClear(false)
		,	persistent(false)
		{
			clearColor[0] =
			clearColor[1] =
			clearColor[2] =
			clearColor[3] = 0.0f;
		}
	};

	Ref< render::ScreenRenderer > m_screenRenderer;
	Ref< PostProcessTargetPool > m_targetPool;
	SmallMap< render::handle_t, Target > m_targets;
	RefArray< PostProcessStep::Instance > m_instances;
	SmallMap< render::handle_t, bool > m_booleanParameters;
	SmallMap< render::handle_t, float > m_scalarParameters;
	SmallMap< render::handle_t, Vector4 > m_vectorParameters;
	SmallMap< render::handle_t, resource::Proxy< render::ITexture > > m_textureParameters;
	Ref< render::RenderTargetSet > m_currentTarget;
	bool m_requireHighRange;
	bool m_allTargetsPersistent;
	Semaphore m_lock;
};

	}
}

#endif	// traktor_world_PostProcess_H
