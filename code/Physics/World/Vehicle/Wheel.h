/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Object.h"
#include "Core/Math/Transform.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class WheelData;

class T_DLLCLASS Wheel : public Object
{
	T_RTTI_CLASS;

public:
	Ref< const WheelData > data;
	float steer;
	float angle;
	float velocity;
	Vector4 center;
	Vector4 direction;
	Vector4 directionPerp;
	float suspensionLength;
	bool contact;
	float contactFudge;
	int32_t contactMaterial;
	Vector4 contactPosition;
	Vector4 contactNormal;
	Vector4 contactVelocity;
	bool sliding;
	float suspensionForce;
	float grip;

	explicit Wheel(const WheelData* data);

	const WheelData* getData() const { return data; }

	float getSteer() const { return steer; }

	float getAngle() const { return angle; }

	float getVelocity() const { return velocity; }

	const Vector4& getCenter() const { return center; }

	const Vector4& getDirection() const { return direction; }

	const Vector4& getDirectionPerp() const { return directionPerp; }

	float getSuspensionLength() const { return suspensionLength; }

	bool getContact() const { return contact; }

	const Vector4& getContactPosition() const { return contactPosition; }

	const Vector4& getContactNormal() const { return contactNormal; }

	const Vector4& getContactVelocity() const { return contactVelocity; }

	bool getSliding() const { return sliding; }

	Transform getTransform() const;
};

}
