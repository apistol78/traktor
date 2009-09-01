#ifndef traktor_scene_PerspectiveRenderControl_H
#define traktor_scene_PerspectiveRenderControl_H

#include "Scene/Editor/ISceneRenderControl.h"
#include "Core/Heap/Ref.h"
#include "Core/Timer/Timer.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Vector4.h"
#include "World/WorldRenderView.h"
#include "Ui/Point.h"

namespace traktor
{
	namespace ui
	{

class Widget;
class Event;

	}

	namespace render
	{

class IRenderView;
class PrimitiveRenderer;

	}

	namespace world
	{

class WorldRenderer;
class Entity;

	}

	namespace scene
	{

class SceneEditorContext;
class Camera;
class EntityAdapter;

class PerspectiveRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS(PerspectiveRenderControl)

public:
	PerspectiveRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	virtual void destroy();

	virtual void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	virtual bool handleCommand(const ui::Command& command);

	virtual void update();

private:
	Ref< SceneEditorContext > m_context;
	Ref< ui::Widget > m_renderWidget;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< world::WorldRenderSettings > m_worldRenderSettings;
	Ref< world::WorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderView;
	float m_fieldOfView;
	Timer m_timer;
	ui::Point m_mousePosition;
	int m_mouseButton;
	bool m_modifyCamera;
	bool m_modifyAlternative;
	Ref< Camera > m_camera;
	RefArray< EntityAdapter > m_modifyEntities;
	ui::Size m_dirtySize;

	void updateWorldRenderer();

	void updateWorldRenderView();

	EntityAdapter* pickEntity(const ui::Point& position) const;

	void eventButtonDown(ui::Event* event);

	void eventButtonUp(ui::Event* event);

	void eventMouseMove(ui::Event* event);

	void eventMouseWheel(ui::Event* event);

	void eventSize(ui::Event* event);

	void eventPaint(ui::Event* event);
};

	}
}

#endif	// traktor_scene_PerspectiveRenderControl_H
