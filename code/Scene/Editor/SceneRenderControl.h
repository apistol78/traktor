#ifndef traktor_scene_SceneRenderControl_H
#define traktor_scene_SceneRenderControl_H

#include "Core/Heap/Ref.h"
#include "Ui/Widget.h"
#include "Core/Timer/Timer.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Vector4.h"
#include "World/WorldRenderView.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Command;

	}

	namespace render
	{

class IRenderView;
class RenderTargetSet;
class PrimitiveRenderer;

	}

	namespace world
	{

class WorldRenderSettings;
class WorldRenderer;
class PostProcessSettings;
class PostProcess;
class Entity;

	}

	namespace scene
	{

class SceneEditorContext;
class Camera;
class IModifier;
class EntityAdapter;

class T_DLLCLASS SceneRenderControl : public ui::Widget
{
	T_RTTI_CLASS(SceneRenderControl)

public:
	SceneRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	void destroy();

	void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	void setPostProcessSettings(world::PostProcessSettings* postProcessSettings);

	bool handleCommand(const ui::Command& command);

private:
	Ref< SceneEditorContext > m_context;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderTargetSet > m_renderTargetSet;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	Ref< world::WorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderView;
	Ref< world::PostProcessSettings > m_postProcessSettings;
	Ref< world::PostProcess > m_postProcess;
	Ref< ui::EventHandler > m_idleHandler;
	Timer m_timer;
	ui::Point m_mousePosition;
	int m_mouseButton;
	bool m_modifyCamera;
	bool m_modifyAlternative;
	RefArray< EntityAdapter > m_modifyEntities;
	ui::Size m_dirtySize;
	float m_lastDeltaTime;
	float m_lastPhysicsTime;

	void updateWorldRenderer();

	void updatePostProcess();

	EntityAdapter* pickEntity(const ui::Point& position) const;

	void eventButtonDown(ui::Event* event);

	void eventButtonUp(ui::Event* event);

	void eventMouseMove(ui::Event* event);

	void eventMouseWheel(ui::Event* event);

	void eventSize(ui::Event* event);

	void eventPaint(ui::Event* event);

	void eventIdle(ui::Event* event);
};

	}
}

#endif	// traktor_scene_SceneRenderControl_H
