/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "ufbx.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"

namespace traktor::model
{

Vector2 convertVector2(const ufbx_vec2& v);

Vector4 convertPosition(const ufbx_vec3& v);

Vector4 convertNormal(const ufbx_vec3& v);

Matrix44 convertMatrix(const ufbx_matrix& m);

Color4f convertColor(const ufbx_vec4& c);

Matrix44 calculateAxisTransform(ufbx_coordinate_axes axisSystem);

}
