/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::scene
{

/*! Modifier anchor point.
 * \ingroup Scene
 *
 * An anchor represents a point of interest that modifiers
 * can manipulate. The default implementation wraps an entity
 * transform, but custom anchors can represent sub-entity
 * features such as faces, vertices or control points.
 */
class T_DLLCLASS IModifierAnchor : public Object
{
	T_RTTI_CLASS;

public:
	virtual Transform getTransform() const = 0;

	virtual void setTransform(const Transform& transform) = 0;
};

}
