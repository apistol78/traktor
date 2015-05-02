#ifndef traktor_scene_DefaultRenderControl_H
#define traktor_scene_DefaultRenderControl_H

#include "Scene/Editor/ISceneRenderControl.h"

namespace traktor
{
	namespace ui
	{

class Container;
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

	virtual void destroy();

	virtual void updateWorldRenderer();

	virtual void setAspect(float aspect);

	virtual void setQuality(world::Quality postProcessQuality, world::Quality shadowQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality);

	virtual bool handleCommand(const ui::Command& command);

	virtual void update();

	virtual bool hitTest(const ui::Point& position) const;

	virtual bool calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const;

	virtual bool calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const;

	virtual void moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta);

	virtual void showSelectionRectangle(const ui::Rect& rect);

private:
	Ref< SceneEditorContext > m_context;
	int32_t m_cameraId;
	int32_t m_viewId;
	Ref< ui::Container > m_container;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::ToolBarButton > m_toolToggleGrid;
	Ref< ui::custom::ToolBarButton > m_toolToggleGuide;
	Ref< ui::custom::ToolBarButton > m_toolToggleFollowEntity;
	Ref< ui::custom::ToolBarButton > m_toolToggleLookAtEntity;
	Ref< ui::custom::ToolBarDropDown > m_toolView;
	Ref< ui::custom::ToolBarDropDown > m_toolAspect;
	Ref< ui::custom::ToolBarDropDown > m_toolPostProcess;
	Ref< ui::custom::ToolBarDropDown > m_toolShadows;
	Ref< ui::custom::ToolBarDropDown > m_toolAO;
	Ref< ui::custom::ToolBarDropDown > m_toolAA;
	Ref< ISceneRenderControl > m_renderControl;

	bool createRenderControl(int32_t type);

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);
};

	}
}

#endif	// traktor_scene_DefaultRenderControl_H
