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

Matrix44 convertMatrix(const cgltf_float* m)
{
	// GLTF matrices are column-major, same as engine
	return Matrix44(
		Vector4(m[0], m[1], m[2], m[3]),
		Vector4(m[4], m[5], m[6], m[7]),
		Vector4(m[8], m[9], m[10], m[11]),
		Vector4(m[12], m[13], m[14], m[15]));
}

Color4f convertColor(const cgltf_float* c, cgltf_size count)
{
	if (count >= 4)
		return Color4f(c[0], c[1], c[2], c[3]);
	else if (count >= 3)
		return Color4f(c[0], c[1], c[2], 1.0f);
	else if (count >= 1)
		return Color4f(c[0], c[0], c[0], 1.0f);
	else
		return Color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

Matrix44 getNodeTransform(const cgltf_node* node)
{
	cgltf_float transform[16];
	cgltf_node_transform_local(node, transform);
	return convertMatrix(transform);
}

Matrix44 calculateGltfAxisTransform()
{
	// GLTF uses Y-up, right-handed coordinate system (+X right, +Y up, +Z toward viewer)
	// Engine also uses right-handed coordinate system but with different Z orientation
	// Based on FBX implementation: leftHanded = false, scale = -1.0f
	// For Y-up case (upAxis = 1), the FBX transform is:
	//   sign * scale, 0.0f, 0.0f, 0.0f,
	//   0.0f, sign, 0.0f, 0.0f,
	//   0.0f, 0.0f, 1.0f, 0.0f,
	//   0.0f, 0.0f, 0.0f, 1.0f
	// Where sign = 1.0f (positive Y up) and scale = -1.0f (right-handed)

	//int upAxis = 1;
	int upSign = 1;

	int frontAxis = 2;
	int frontSign = 1;

	Matrix44 axisTransform = Matrix44::identity();

	bool leftHanded = false;

	const float sign = upSign < 0 ? -1.0f : 1.0f;
	const float scale = leftHanded ? 1.0f : -1.0f;

	axisTransform = Matrix44(
		sign * scale,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		sign,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		1.0f);

	return axisTransform;
}

}