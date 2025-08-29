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
#include "Core/Math/Vector4.h"

namespace traktor::model
{

/*! Convert cgltf vector2 to engine Vector2. */
Vector2 convertVector2(const cgltf_float* v);

/*! Convert cgltf vector3 to engine position vector (w=1). */
Vector4 convertPosition(const cgltf_float* v);

/*! Convert cgltf vector3 to engine normal vector (w=0). */
Vector4 convertNormal(const cgltf_float* v);

/*! Convert cgltf 4x4 matrix to engine Matrix44. */
Matrix44 convertMatrix(const cgltf_float* m);

/*! Convert cgltf color to engine Color4f. */
Color4f convertColor(const cgltf_float* c, cgltf_size count);

/*! Get node's local transform matrix. */
Matrix44 getNodeTransform(const cgltf_node* node);

/*! Calculate coordinate system transform for GLTF (Y-up, right-handed) to engine coordinates. */
Matrix44 calculateGltfAxisTransform();

}