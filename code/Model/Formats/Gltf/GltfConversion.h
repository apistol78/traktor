/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "cgltf.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector2.h"

namespace traktor::model
{

Vector2 convertVector2(const cgltf_float* v);

Vector4 convertPosition(const cgltf_float* v);

Vector4 convertNormal(const cgltf_float* v);

Vector4 convertVector4(const cgltf_float* v);

Matrix44 convertMatrix(const cgltf_float* m);

Color4f convertColor3(const cgltf_float* c);

Color4f convertColor4(const cgltf_float* c);

}
