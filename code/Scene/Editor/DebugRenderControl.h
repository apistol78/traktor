#ifndef traktor_scene_DebugRenderControl_H
#define traktor_scene_DebugRenderControl_H

#include "Core/Math/Vector2.h"
#include "Resource/Proxy.h"
#include "Scene/Editor/ISceneRenderControl.h"
#include "Ui/Point.h"
#include "Ui/Size.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace ui
	{

class Widget;

	}

	namespace render
	{

class IRenderView;
class ITexture;
class ScreenRenderer;
class Shader;

	}

	namespace scene
	{

class SceneEditorContext;

class DebugRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	DebugRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void updateWorldRenderer() T_OVERRIDE T_FINAL;

	virtual void setAspect(float aspect) T_OVERRIDE T_FINAL;

	virtual void setQuality(world::Quality imageProcessQuality, world::Quality shadowQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void update() T_OVERRIDE T_FINAL;

	virtual bool hitTest(const ui::Point& position) const T_OVERRIDE T_FINAL;

	virtual bool calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const T_OVERRIDE T_FINAL;

	virtual bool calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const T_OVERRIDE T_FINAL;

	virtual void moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta) T_OVERRIDE T_FINAL;

	virtual void showSelectionRectangle(const ui::Rect& rect) T_OVERRIDE T_FINAL;

private:
	Ref< SceneEditorContext > m_context;
	Ref< ui::Widget > m_renderWidget;
	Ref< render::IRenderView > m_renderView;
	Ref< render::ScreenRenderer > m_screenRenderer;
	resource::Proxy< render::Shader > m_shader;
	ui::Size m_dirtySize;
	Vector2 m_renderOffset;
	float m_renderScale;
	ui::Point m_moveMouseOrigin;
	Vector2 m_moveRenderOffset;

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

#endif	// traktor_scene_DebugRenderControl_H
