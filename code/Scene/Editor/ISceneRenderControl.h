/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Frustum.h"
#include "Core/Object.h"
#include "Render/Types.h"
#include "Ui/Rect.h"
#include "World/IWorldRenderer.h"
#include "World/WorldTypes.h"

#include <vector>

namespace traktor::ui
{

class Command;

}

namespace traktor::world
{

class IDebugOverlay;

}

namespace traktor::scene
{

class ISceneRenderControl : public Object
{
	T_RTTI_CLASS;

public:
	enum MoveCameraMode
	{
		McmRotate,
		McmMove,
		McmMoveXZ,
		McmMoveXY
	};

	virtual void destroy() = 0;

	virtual void setWorldRendererType(const TypeInfo& worldRendererType) = 0;

	virtual void setAspect(float aspect) = 0;

	virtual void setQuality(const world::QualitySettings& qualitySettings) = 0;

	virtual void setDebugOverlay(world::IDebugOverlay* overlay) = 0;

	virtual void setDebugOverlayAlpha(float alpha, float mip) = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;

	virtual void update() = 0;

	virtual bool hitTest(const ui::Point& position) const = 0;

	virtual bool calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const = 0;

	virtual bool calculateFrustum(const ui::Rect& rc, Frustum& outWorldFrustum) const = 0;

	virtual void moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta) = 0;

	virtual void showSelectionRectangle(const ui::Rect& rect) = 0;
};

}
