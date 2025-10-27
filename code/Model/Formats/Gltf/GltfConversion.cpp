/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#define CGLTF_IMPLEMENTATION
#include "Model/Formats/Gltf/GltfConversion.h"

namespace traktor::model
{

Vector2 convertVector2(const cgltf_float* v)
{
	return Vector2(v[0], v[1]);
}

Vector4 convertPosition(const cgltf_float* v)
{
	return Vector4(v[0], v[1], v[2], 1.0f);
}

Vector4 convertNormal(const cgltf_float* v)
{
	return Vector4(v[0], v[1], v[2], 0.0f);
}

Vector4 convertVector4(const cgltf_float* v)
{
	return Vector4(v[0], v[1], v[2], v[3]);
}

Matrix44 convertMatrix(const cgltf_float* m)
{
	// glTF matrices are column-major
	return Matrix44(
		Vector4(m[0], m[1], m[2], m[3]),
		Vector4(m[4], m[5], m[6], m[7]),
		Vector4(m[8], m[9], m[10], m[11]),
		Vector4(m[12], m[13], m[14], m[15]));
}

Color4f convertColor3(const cgltf_float* c)
{
	return Color4f(c[0], c[1], c[2], 1.0f);
}

Color4f convertColor4(const cgltf_float* c)
{
	return Color4f(c[0], c[1], c[2], c[3]);
}

}
