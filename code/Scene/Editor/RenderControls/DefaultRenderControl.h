/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/ISceneRenderControl.h"

namespace traktor::ui
{

class Container;
class MenuItem;
class Slider;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class ToolBarDropDown;
class ToolBarDropMenu;
class Widget;

}

namespace traktor::scene
{

class SceneEditorContext;

class DefaultRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	DefaultRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context, int32_t cameraId, int32_t viewId);

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
	int32_t m_cameraId;
	int32_t m_viewId;
	Ref< ui::Container > m_container;
	Ref< ui::ToolBar > m_toolBar;
	Ref< ui::ToolBarButton > m_toolToggleGrid;
	Ref< ui::ToolBarButton > m_toolToggleGuide;
	Ref< ui::ToolBarDropDown > m_toolView;
	Ref< ui::ToolBarDropDown > m_toolAspect;
	Ref< ui::ToolBarDropMenu > m_toolQualityMenu;
	Ref< ui::ToolBarDropDown > m_toolWorldRenderer;
	Ref< ui::ToolBarDropDown > m_toolDebugOverlay;
	Ref< ui::Slider > m_sliderDebugAlpha;
	Ref< ui::Slider > m_sliderDebugMip;
	Ref< ui::MenuItem > m_menuPostProcess;
	Ref< ui::MenuItem > m_menuMotionBlur;
	Ref< ui::MenuItem > m_menuShadows;
	Ref< ui::MenuItem > m_menuReflections;
	Ref< ui::MenuItem > m_menuAO;
	Ref< ui::MenuItem > m_menuAA;
	Ref< ISceneRenderControl > m_renderControl;

	bool createRenderControl(int32_t type);

	void eventResize(ui::SizeEvent* event);

	void eventSliderDebugChange(ui::ContentChangeEvent* event);

	void eventToolClick(ui::ToolBarButtonClickEvent* event);
};

}
