#ifndef traktor_scene_OrthogonalRenderControl_H
#define traktor_scene_OrthogonalRenderControl_H

#include "Core/RefArray.h"
#include "Core/Math/Color.h"
#include "Core/Math/Matrix44.h"
#include "Core/Timer/Timer.h"
#include "Scene/Editor/ISceneRenderControl.h"
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

	}

	namespace scene
	{

class SceneEditorContext;
class EntityAdapter;

class OrthogonalRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	enum ViewPlane
	{
		PositiveX = 0,
		NegativeX = 1,
		PositiveY = 2,
		NegativeY = 3,
		PositiveZ = 4,
		NegativeZ = 5
	};

	OrthogonalRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context, ViewPlane viewPlane);

	void destroy();

	virtual void updateWorldRenderer();

	virtual bool handleCommand(const ui::Command& command);

	virtual void update();

private:
	Ref< SceneEditorContext > m_context;
	Ref< ui::Widget > m_renderWidget;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< world::WorldRenderer > m_worldRenderer;
	bool m_gridEnable;
	bool m_guideEnable;
	Color m_colorClear;
	Color m_colorGrid;
	Color m_colorRef;
	Color m_colorCamera;
	Timer m_timer;
	ui::Point m_mousePosition;
	int m_mouseButton;
	bool m_modifyCamera;
	bool m_modifyAlternative;
	bool m_modifyBegun;
	RefArray< EntityAdapter > m_modifyEntities;
	ViewPlane m_viewPlane;
	float m_viewFarZ;
	float m_magnification;
	float m_cameraX;
	float m_cameraY;
	ui::Size m_dirtySize;

	void updateSettings();

	Matrix44 getProjectionTransform() const;

	Matrix44 getViewTransform() const;

	Ref< EntityAdapter > pickEntity(const ui::Point& position) const;

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
