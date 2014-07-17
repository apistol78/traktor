#ifndef traktor_scene_PerspectiveRenderControl_H
#define traktor_scene_PerspectiveRenderControl_H

#include "Core/RefArray.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Matrix44.h"
#include "Core/Timer/Timer.h"
#include "Scene/Editor/ISceneRenderControl.h"
#include "Scene/Editor/RenderControlModel.h"
#include "Ui/Rect.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace ui
	{

class Container;
class Widget;

	}

	namespace render
	{

class IRenderView;
class RenderTargetSet;
class PrimitiveRenderer;

	}

	namespace world
	{

class IWorldRenderer;
class Entity;

	}

	namespace scene
	{

class Camera;
class SceneEditorContext;

class PerspectiveRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	PerspectiveRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context, int32_t cameraId);

	virtual void destroy();

	virtual void updateWorldRenderer();

	virtual void setAspect(float aspect);

	virtual void setQuality(world::Quality shadowQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality);

	virtual bool handleCommand(const ui::Command& command);

	virtual void update();

	virtual bool hitTest(const ui::Point& position) const;

	virtual bool calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const;

	virtual bool calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const;

	virtual void moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta);

	virtual void showSelectionRectangle(const ui::Rect& rect);

private:
	Ref< SceneEditorContext > m_context;
	Ref< ui::Container > m_containerAspect;
	Ref< ui::Widget > m_renderWidget;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< world::IWorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderView;
	world::WorldRenderSettings m_worldRenderSettings;
	world::Quality m_shadowQuality;
	world::Quality m_ambientOcclusionQuality;
	world::Quality m_antiAliasQuality;
	RenderControlModel m_model;
	bool m_gridEnable;
	bool m_guideEnable;
	bool m_postProcessEnable;
	Color4ub m_colorClear;
	Color4ub m_colorGrid;
	Color4ub m_colorRef;
	float m_fieldOfView;
	float m_mouseWheelRate;
	int32_t m_multiSample;
	bool m_invertPanY;
	Timer m_timer;
	Ref< Camera > m_camera;
	ui::Rect m_selectionRectangle;
	ui::Size m_dirtySize;

	void updateSettings();

	void updateWorldRenderView();

	Matrix44 getProjectionTransform() const;

	Matrix44 getViewTransform() const;

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);
};

	}
}

#endif	// traktor_scene_PerspectiveRenderControl_H
