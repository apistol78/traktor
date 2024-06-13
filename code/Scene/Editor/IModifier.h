/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class PrimitiveRenderer;

}

namespace traktor::ui
{

class Command;

}

namespace traktor::scene
{

class TransformChain;

/*! Selection modifier abstraction. */
class T_DLLCLASS IModifier : public Object
{
	T_RTTI_CLASS;

public:
	struct CursorMovedResult
	{
		bool hot;
		bool redraw;
	};

	/*! \name Notifications */
	//\{

	virtual bool activate() = 0;

	virtual void deactivate() = 0;

	virtual void selectionChanged() = 0;

	virtual void buttonDown() = 0;

	virtual CursorMovedResult cursorMoved(
		const TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection
	) = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;

	//\}

	/*! \name Modifications */
	//\{

	virtual bool begin(
		const TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		int32_t mouseButton
	) = 0;

	virtual void apply(
		const TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		const Vector4& screenDelta,
		const Vector4& viewDelta,
		bool snapOverrideEnable
	) = 0;

	virtual void end(const TransformChain& transformChain) = 0;

	//\}

	/*! \name Preview */
	//\{

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) const = 0;

	//\}
};

}
