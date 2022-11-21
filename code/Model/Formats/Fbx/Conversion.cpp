/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Formats/Fbx/Conversion.h"

namespace traktor::model
{

Vector2 convertVector2(const FbxVector2& v)
{
	return Vector2(float(v[0]), float(v[1]));
}

Vector4 convertVector4(const FbxVector4& v)
{
	return Vector4(float(v[0]), float(v[1]), float(v[2]), float(v[3]));
}

Vector4 convertNormal(const FbxVector4& v)
{
	return Vector4(float(v[0]), float(v[1]), float(v[2]), 0.0f);
}

Vector4 convertPosition(const FbxVector4& v)
{
	return Vector4(float(v[0]), float(v[1]), float(v[2]), 1.0f);
}

Quaternion convertQuaternion(const FbxQuaternion& v)
{
	return Quaternion(float(v[0]), float(v[1]), float(v[2]), float(v[3]));
}

Matrix44 convertMatrix(const FbxMatrix& m)
{
	return Matrix44(
		convertVector4(m.GetRow(0)),
		convertVector4(m.GetRow(1)),
		convertVector4(m.GetRow(2)),
		convertVector4(m.GetRow(3))
	);
}

Matrix44 convertMatrix(const FbxAMatrix& m)
{
	return Matrix44(
		convertVector4(m.GetRow(0)),
		convertVector4(m.GetRow(1)),
		convertVector4(m.GetRow(2)),
		convertVector4(m.GetRow(3))
	);
}

traktor::Transform convertTransform(const FbxAMatrix& m)
{
	return traktor::Transform(
		convertVector4(m.GetT()),
		convertQuaternion(m.GetQ())
	);
}

Color4f convertColor(const FbxColor& c)
{
	return Color4f(float(c.mRed), float(c.mGreen), float(c.mBlue), float(c.mAlpha));
}

Matrix44 calculateAxisTransform(const FbxAxisSystem& axisSystem)
{
	Matrix44 axisTransform = Matrix44::identity();

	int upSign;
	FbxAxisSystem::EUpVector up = axisSystem.GetUpVector(upSign);

	int frontSign;
	FbxAxisSystem::EFrontVector front = axisSystem.GetFrontVector(frontSign);

	auto coordSystem = axisSystem.GetCoorSystem();
	bool leftHanded = bool(coordSystem == FbxAxisSystem::eLeftHanded);

	float sign = upSign < 0 ? -1.0f : 1.0f;
	float scale = leftHanded ? 1.0f : -1.0f;

	switch (up)
	{
	case FbxAxisSystem::eXAxis:
		axisTransform = Matrix44(
			0.0f, sign, 0.0f, 0.0f,
			-sign, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, scale, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;

	case FbxAxisSystem::eYAxis:
		axisTransform = Matrix44(
			sign * scale, 0.0f, 0.0f, 0.0f,
			0.0f, sign, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;

	case FbxAxisSystem::eZAxis:
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
