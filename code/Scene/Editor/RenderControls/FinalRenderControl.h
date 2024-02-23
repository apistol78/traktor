/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Matrix44.h"
#include "Core/Timer/Timer.h"
#include "Resource/Proxy.h"
#include "Scene/Editor/ISceneRenderControl.h"
#include "Scene/Editor/RenderControlModel.h"
#include "Ui/Rect.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"

namespace traktor::ui
{

class Container;
class Widget;

}

namespace traktor::render
{

class IRenderView;
class RenderContext;
class RenderGraph;
class ScreenRenderer;

}

namespace traktor::world
{

class IWorldRenderer;
class Entity;

}

namespace traktor::scene
{

class Camera;
class Scene;
class SceneEditorContext;

class FinalRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	FinalRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context, int32_t cameraId, const TypeInfo& worldRendererType);

	virtual void destroy() override final;

	virtual void setWorldRendererType(const TypeInfo& worldRendererType) override final;

	virtual void setAspect(float aspect) override final;

	virtual void setQuality(world::Quality imageProcess, world::Quality shadows, world::Quality reflections, world::Quality motionBlur, world::Quality ambientOcclusion, world::Quality antiAlias) override final;

	virtual void setDebugOverlay(world::IDebugOverlay* overlay) override final;

	virtual void setDebugOverlayAlpha(float alpha, float mip) override final;
	
	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void update() override final;

	virtual bool hitTest(const ui::Point& position) const override final;

	virtual bool calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const override final;

	virtual bool calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const override final;

	virtual void moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta) override final;

	virtual void showSelectionRectangle(const ui::Rect& rect) override final;

private:
	Ref< SceneEditorContext > m_context;
	Ref< ui::Container > m_containerAspect;
	Ref< ui::Widget > m_renderWidget;
	Ref< render::IRenderView > m_renderView;
	Ref< render::RenderContext > m_renderContext;
	Ref< render::RenderGraph > m_renderGraph;
	resource::Proxy< scene::Scene > m_sceneInstance;
	Ref< render::ScreenRenderer > m_screenRenderer;
	const TypeInfo* m_worldRendererType = nullptr;
	Ref< world::IWorldRenderer > m_worldRenderer;
	uint32_t m_worldRendererHash = 0;
	world::WorldRenderView m_worldRenderView;
	world::WorldRenderSettings m_worldRenderSettings;
	world::Quality m_imageProcessQuality;
	world::Quality m_shadowQuality;
	world::Quality m_reflectionsQuality;
	world::Quality m_motionBlurQuality;
	world::Quality m_ambientOcclusionQuality;
	world::Quality m_antiAliasQuality;
	RenderControlModel m_model;
	Ref< world::IDebugOverlay > m_overlay;
	float m_overlayAlpha = 1.0f;
	float m_overlayMip = 0.0f;
	Color4ub m_colorClear;
	float m_fieldOfView;
	float m_mouseWheelRate;
	int32_t m_multiSample;
	bool m_invertPanY;
	Timer m_timer;
	Ref< Camera > m_camera;
	ui::Size m_dirtySize;

	void updateWorldRenderer();

	void updateSettings();

	Matrix44 getProjectionTransform() const;

	Matrix44 getViewTransform() const;

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);

	void eventKeyDown(ui::KeyDownEvent* event);

	void eventKeyUp(ui::KeyUpEvent* event);

	void eventPaint(ui::PaintEvent* event);
};

}
