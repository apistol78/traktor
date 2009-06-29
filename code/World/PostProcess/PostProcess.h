#ifndef traktor_world_PostProcess_H
#define traktor_world_PostProcess_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class RenderSystem;
class RenderView;
class RenderTargetSet;
class ScreenRenderer;

	}

	namespace world
	{

class PostProcessSettings;
class WorldRenderView;

/*! \brief Frame buffer post processing system.
 * \ingroup World
 *
 * Predefined targets:
 * 0 - Frame buffer, write only.
 * 1 - Source color buffer, read only.
 * 2 - Source depth buffer, read only.
 */
class T_DLLCLASS PostProcess : public Object
{
	T_RTTI_CLASS(PostProcess)

public:
	bool create(
		PostProcessSettings* settings,
		resource::IResourceManager* resourceManager,
		render::RenderSystem* renderSystem,
		uint32_t screenWidth,
		uint32_t screenHeight
	);

	void destroy();

	bool render(
		const WorldRenderView& worldRenderView,
		render::RenderView* renderView,
		render::RenderTargetSet* frameBuffer,
		render::RenderTargetSet* depthBuffer,
		float deltaTime
	);

	void setTarget(render::RenderView* renderView, uint32_t id);

	Ref< render::RenderTargetSet >& getTargetRef(uint32_t id);

	const std::map< uint32_t, Ref< render::RenderTargetSet > >& getTargets() const;

private:
	Ref< PostProcessSettings > m_settings;
	Ref< render::ScreenRenderer > m_screenRenderer;
	std::map< uint32_t, Ref< render::RenderTargetSet > > m_targets;
	Ref< render::RenderTargetSet > m_currentTarget;
};

	}
}

#endif	// traktor_world_PostProcess_H
