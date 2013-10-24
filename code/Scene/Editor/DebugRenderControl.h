#ifndef traktor_scene_DebugRenderControl_H
#define traktor_scene_DebugRenderControl_H

#include "Core/Math/Vector2.h"
#include "Scene/Editor/ISceneRenderControl.h"
#include "Ui/Point.h"
#include "Ui/Size.h"

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
class ITexture;
class PrimitiveRenderer;

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

	void destroy();

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
	Ref< ui::Widget > m_renderWidget;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	ui::Size m_dirtySize;
	Vector2 m_renderOffset;
	float m_renderScale;
	ui::Point m_moveMouseOrigin;
	Vector2 m_moveRenderOffset;

	void eventButtonDown(ui::Event* event);

	void eventButtonUp(ui::Event* event);

	void eventDoubleClick(ui::Event* event);

	void eventMouseMove(ui::Event* event);

	void eventMouseWheel(ui::Event* event);

	void eventSize(ui::Event* event);

	void eventPaint(ui::Event* event);
};

	}
}

#endif	// traktor_scene_DebugRenderControl_H
