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

class IRenderTargetSet;
class IRenderView;
class PrimitiveRenderer;
class RenderContext;
class RenderGraph;

}

namespace traktor::world
{

class IWorldRenderer;
class Entity;

}

namespace traktor::scene
{

class SceneEditorContext;

class CameraRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	CameraRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context, const TypeInfo& worldRendererType);

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
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
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
	bool m_gridEnable;
	bool m_guideEnable;
	Color4ub m_colorClear;
	Color4ub m_colorGrid;
	Color4ub m_colorRef;
	int32_t m_multiSample;
	bool m_invertPanY;
	RefArray< EntityAdapter > m_cameraEntities;
	Timer m_timer;
	ui::Rect m_selectionRectangle;
	ui::Size m_dirtySize;

	void updateWorldRenderer();

	void updateSettings();

	void eventPaint(ui::PaintEvent* event);
};

}
