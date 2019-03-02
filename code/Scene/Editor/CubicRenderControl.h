#pragma once

#include "Core/RefArray.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Timer/Timer.h"
#include "Resource/Proxy.h"
#include "Scene/Editor/ISceneRenderControl.h"
#include "Ui/Rect.h"
#include "Ui/Events/AllEvents.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace render
	{

class IndexBuffer;
class ICubeTexture;
class IRenderView;
class RenderTargetSet;
class Shader;
class VertexBuffer;

	}

	namespace ui
	{

class Container;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class Widget;

	}

	namespace world
	{

class IWorldRenderer;
class Entity;

	}

	namespace scene
	{

class SceneEditorContext;

class CubicRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	CubicRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	virtual void destroy() override final;

	virtual void updateWorldRenderer() override final;

	virtual void setAspect(float aspect) override final;

	virtual void setQuality(world::Quality imageProcessQuality, world::Quality shadowQuality, world::Quality reflectionsQuality, world::Quality motionBlurQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void update() override final;

	virtual bool hitTest(const ui::Point& position) const override final;

	virtual bool calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const override final;

	virtual bool calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const override final;

	virtual void moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta) override final;

	virtual void showSelectionRectangle(const ui::Rect& rect) override final;

private:
	Ref< SceneEditorContext > m_context;
	Ref< ui::Container > m_container;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::Widget > m_renderWidget;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderTargetSet > m_renderTargetSet;
	Ref< render::ICubeTexture > m_cubeMapTexture;
	Ref< drawing::Image > m_cubeImages[6];
	Ref< render::VertexBuffer > m_vertexBuffer;
	Ref< render::IndexBuffer > m_indexBuffer;
	resource::Proxy< render::Shader > m_shader;
	Ref< world::IWorldRenderer > m_worldRenderer;
	world::WorldRenderView m_worldRenderView;
	world::WorldRenderSettings m_worldRenderSettings;
	world::Quality m_imageProcessQuality;
	world::Quality m_shadowQuality;
	world::Quality m_reflectionsQuality;
	world::Quality m_motionBlurQuality;
	world::Quality m_ambientOcclusionQuality;
	world::Quality m_antiAliasQuality;
	ui::Point m_mousePosition0;
	ui::Point m_mousePosition;
	int32_t m_mouseButton;
	Quaternion m_previewOrientation;
	Timer m_timer;
	ui::Size m_dirtySize;

	void capture(const Vector4& capture);

	void eventToolClick(ui::ToolBarButtonClickEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);
};

	}
}

