/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_ISceneRenderControl_H
#define traktor_scene_ISceneRenderControl_H

#include "Core/Object.h"
#include "Core/Math/Frustum.h"
#include "World/WorldTypes.h"

namespace traktor
{
	namespace ui
	{

class Command;
class Point;
class Rect;

	}

	namespace scene
	{

class ISceneRenderControl : public Object
{
	T_RTTI_CLASS;

public:
	enum MoveCameraMode
	{
		McmRotate = 0,
		McmMoveXZ = 1,
		McmMoveXY = 2
	};

	virtual void destroy() = 0;

	virtual void updateWorldRenderer() = 0;

	virtual void setAspect(float aspect) = 0;

	virtual void setQuality(
		world::Quality imageProcessQuality,
		world::Quality shadowQuality,
		world::Quality reflectionsQuality,
		world::Quality motionBlurQuality,
		world::Quality ambientOcclusionQuality,
		world::Quality antiAliasQuality
	) = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;

	virtual void update() = 0;

	virtual bool hitTest(const ui::Point& position) const = 0;

	virtual bool calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const = 0;

	virtual bool calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const = 0;

	virtual void moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta) = 0;

	virtual void showSelectionRectangle(const ui::Rect& rect) = 0;
};

	}
}

#endif	// traktor_scene_ISceneRenderControl_H
