/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_CameraRenderControl_H
#define traktor_scene_CameraRenderControl_H

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

class SceneEditorContext;

class CameraRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	CameraRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void updateWorldRenderer() T_OVERRIDE T_FINAL;

	virtual void setAspect(float aspect) T_OVERRIDE T_FINAL;

	virtual void setQuality(world::Quality imageProcessQuality, world::Quality shadowQuality, world::Quality reflectionsQuality, world::Quality motionBlurQuality, world::Quality ambientOcclusionQuality, world::Quality antiAliasQuality) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void update() T_OVERRIDE T_FINAL;

	virtual bool hitTest(const ui::Point& position) const T_OVERRIDE T_FINAL;

	virtual bool calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const T_OVERRIDE T_FINAL;

	virtual bool calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const T_OVERRIDE T_FINAL;

	virtual void moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta) T_OVERRIDE T_FINAL;

	virtual void showSelectionRectangle(const ui::Rect& rect) T_OVERRIDE T_FINAL;

private:
	Ref< SceneEditorContext > m_context;
	Ref< ui::Container > m_containerAspect;
	Ref< ui::Widget > m_renderWidget;
	Ref< render::IRenderView > m_renderView;
	Ref< render::PrimitiveRenderer > m_primitiveRenderer;
	Ref< world::IWorldRenderer > m_worldRenderer;
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

	void updateSettings();

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);
};

	}
}

#endif	// traktor_scene_CameraRenderControl_H
