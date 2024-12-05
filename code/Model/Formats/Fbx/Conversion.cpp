/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Formats/Fbx/Conversion.h"

namespace traktor::model
{

Vector2 convertVector2(const ufbx_vec2& v)
{
	return Vector2(v.x, v.y);
}

Vector4 convertPosition(const ufbx_vec3& v)
{
	return Vector4(v.x, v.y, v.z, 1.0f);
}

Vector4 convertNormal(const ufbx_vec3& v)
{
	return Vector4(v.x, v.y, v.z, 0.0f);
}

Matrix44 convertMatrix(const ufbx_matrix& m)
{
	return Matrix44(
		Vector4(m.cols[0].x, m.cols[0].y, m.cols[0].z, 0.0f),
		Vector4(m.cols[1].x, m.cols[1].y, m.cols[1].z, 0.0f),
		Vector4(m.cols[2].x, m.cols[2].y, m.cols[2].z, 0.0f),
		Vector4(m.cols[3].x, m.cols[3].y, m.cols[3].z, 1.0f)
	);
}

Color4f convertColor(const ufbx_vec4& c)
{
	return Color4f(c.x, c.y, c.z, c.w);
}

Matrix44 calculateAxisTransform(ufbx_coordinate_axes axisSystem)
{
	int upAxis;
	int upSign;

	switch (axisSystem.up)
	{
	case UFBX_COORDINATE_AXIS_POSITIVE_X:
		upAxis = 0;
		upSign = 1;
		break;
	case UFBX_COORDINATE_AXIS_NEGATIVE_X:
		upAxis = 0;
		upSign = -1;
		break;

	case UFBX_COORDINATE_AXIS_POSITIVE_Y:
		upAxis = 1;
		upSign = 1;
		break;
	case UFBX_COORDINATE_AXIS_NEGATIVE_Y:
		upAxis = 1;
		upSign = -1;
		break;

	case UFBX_COORDINATE_AXIS_POSITIVE_Z:
		upAxis = 2;
		upSign = 1;
		break;
	case UFBX_COORDINATE_AXIS_NEGATIVE_Z:
		upAxis = 2;
		upSign = -1;
		break;

	default:
		return Matrix44::identity();
	}

	int frontAxis;
	int frontSign;

	switch (axisSystem.front)
	{
	case UFBX_COORDINATE_AXIS_POSITIVE_X:
		frontAxis = 0;
		frontSign = 1;
		break;
	case UFBX_COORDINATE_AXIS_NEGATIVE_X:
		frontAxis = 0;
		frontSign = -1;
		break;

	case UFBX_COORDINATE_AXIS_POSITIVE_Y:
		frontAxis = 1;
		frontSign = 1;
		break;
	case UFBX_COORDINATE_AXIS_NEGATIVE_Y:
		frontAxis = 1;
		frontSign = -1;
		break;

	case UFBX_COORDINATE_AXIS_POSITIVE_Z:
		frontAxis = 2;
		frontSign = 1;
		break;
	case UFBX_COORDINATE_AXIS_NEGATIVE_Z:
		frontAxis = 2;
		frontSign = -1;
		break;

	default:
		return Matrix44::identity();
	}

	Matrix44 axisTransform = Matrix44::identity();

	bool leftHanded = false;

	const float sign = upSign < 0 ? -1.0f : 1.0f;
	const float scale = leftHanded ? 1.0f : -1.0f;

	switch (upAxis)
	{
	case 0:
		axisTransform = Matrix44(
			0.0f, sign, 0.0f, 0.0f,
			-sign, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, scale, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;

	case 1:
		axisTransform = Matrix44(
			sign * scale, 0.0f, 0.0f, 0.0f,
			0.0f, sign, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;

	case 2:
		axisTransform = Matrix44(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, -sign * scale, 0.0f,
			0.0f, sign, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;
	}

	return axisTransform;
}

}
