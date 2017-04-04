#ifndef traktor_scene_DefaultRenderControl_H
#define traktor_scene_DefaultRenderControl_H

#include "Scene/Editor/ISceneRenderControl.h"

namespace traktor
{
	namespace ui
	{

class Container;
class MenuItem;
class Widget;

		namespace custom
		{

class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class ToolBarDropDown;

		}
	}

	namespace scene
	{

class SceneEditorContext;

class DefaultRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	DefaultRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context, int32_t cameraId, int32_t viewId);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void updateWorldRenderer() T_OVERRIDE T_FINAL;

	virtual void setAspect(float aspect) T_OVERRIDE T_FINAL;

	virtual void setQuality(world::Quality imageProcessQuality, world::Quality shadowQuality, world::Quality motionBlurQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void update() T_OVERRIDE T_FINAL;

	virtual bool hitTest(const ui::Point& position) const T_OVERRIDE T_FINAL;

	virtual bool calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const T_OVERRIDE T_FINAL;

	virtual bool calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const T_OVERRIDE T_FINAL;

	virtual void moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta) T_OVERRIDE T_FINAL;

	virtual void showSelectionRectangle(const ui::Rect& rect) T_OVERRIDE T_FINAL;

private:
	Ref< SceneEditorContext > m_context;
	int32_t m_cameraId;
	int32_t m_viewId;
	Ref< ui::Container > m_container;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::ToolBarButton > m_toolToggleGrid;
	Ref< ui::custom::ToolBarButton > m_toolToggleGuide;
	Ref< ui::custom::ToolBarDropDown > m_toolView;
	Ref< ui::custom::ToolBarDropDown > m_toolAspect;
	Ref< ui::MenuItem > m_menuPostProcess;
	Ref< ui::MenuItem > m_menuMotionBlur;
	Ref< ui::MenuItem > m_menuShadows;
	Ref< ui::MenuItem > m_menuAO;
	Ref< ui::MenuItem > m_menuAA;
	Ref< ISceneRenderControl > m_renderControl;

	bool createRenderControl(int32_t type);

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);
};

	}
}

#endif	// traktor_scene_DefaultRenderControl_H
