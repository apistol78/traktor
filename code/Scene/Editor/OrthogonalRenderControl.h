#ifndef traktor_scene_OrthogonalRenderControl_H
#define traktor_scene_OrthogonalRenderControl_H

#include "Scene/Editor/ISceneRenderControl.h"
#include "Core/Heap/Ref.h"
#include "Core/Timer/Timer.h"

namespace traktor
{
	namespace render
	{

class IRenderView;
class PrimitiveRenderer;

	}

	namespace world
	{

class WorldRenderer;

	}

	namespace scene
	{

class SceneEditorContext;
class EntityAdapter;

class OrthogonalRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS(OrthogonalRenderControl)

public:
	OrthogonalRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context, int32_t viewPlane);

	void destroy();

	virtual void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	virtual bool handleCommand(const ui::Command& command);

private:
	Ref< SceneEditorContext > m_context;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	Ref< world::WorldRenderer > m_worldRenderer;
	Timer m_timer;

	ui::Point m_mousePosition;
	int m_mouseButton;
	bool m_modifyCamera;
	bool m_modifyAlternative;
	RefArray< EntityAdapter > m_modifyEntities;

	int32_t m_viewPlane;
	float m_magnification;
	float m_cameraX;
	float m_cameraY;

	ui::Size m_dirtySize;

	void updateWorldRenderer();

	void eventButtonDown(ui::Event* event);

	void eventButtonUp(ui::Event* event);

	void eventMouseMove(ui::Event* event);

	void eventMouseWheel(ui::Event* event);

	void eventSize(ui::Event* event);

	void eventPaint(ui::Event* event);
};

	}
}

#endif	// traktor_scene_OrthogonalRenderControl_H
