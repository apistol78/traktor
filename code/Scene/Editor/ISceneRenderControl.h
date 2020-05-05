#pragma once

#include <vector>
#include "Core/Object.h"
#include "Core/Math/Frustum.h"
#include "Render/Types.h"
#include "World/WorldTypes.h"

namespace traktor
{
	namespace ui
	{

class Command;
class Point;
class Rect;

	}

	namespace world
	{

class IDebugOverlay;

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

	virtual void setWorldRendererType(const TypeInfo& worldRendererType) = 0;

	virtual void setAspect(float aspect) = 0;

	virtual void setQuality(
		world::Quality imageProcess,
		world::Quality shadows,
		world::Quality reflections,
		world::Quality motionBlur,
		world::Quality ambientOcclusion,
		world::Quality antiAlias
	) = 0;

	virtual void setDebugOverlay(world::IDebugOverlay* overlay) = 0;

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

