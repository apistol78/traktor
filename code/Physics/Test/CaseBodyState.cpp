/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Test/MathCompare.h"
#include "Physics/BodyState.h"
#include "Physics/Test/CaseBodyState.h"

namespace traktor::physics::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.test.CaseBodyState", 0, CaseBodyState, traktor::test::Case)

void CaseBodyState::run()
{
	BodyState S0, S1;

	S0.setTransform(Transform(
		Vector4(-1.0f, -2.0f, -3.0f, 1.0f),
		Quaternion::fromEulerAngles(
			deg2rad(90.0f),
			deg2rad(0.0f),
			deg2rad(0.0f)
		)
	));
	S0.setLinearVelocity(Vector4(-1.0f, -2.0f, -3.0f, 0.0f));
	S0.setAngularVelocity(Vector4(1.0f, 0.0f, 0.0f, 0.0f));

	S1.setTransform(Transform(
		Vector4(1.0f, 2.0f, 3.0f, 1.0f),
		Quaternion::fromEulerAngles(
			deg2rad(0.0f),
			deg2rad(90.0f),
			deg2rad(0.0f)
		)
	));
	S1.setLinearVelocity(Vector4(1.0f, 2.0f, 3.0f, 0.0f));
	S1.setAngularVelocity(Vector4(0.0f, 1.0f, 0.0f, 0.0f));

	BodyState R0 = S0.interpolate(S1, 0.0_simd);
	BodyState R1 = S0.interpolate(S1, 1.0_simd);

	CASE_ASSERT_COMPARE(S0.getTransform(), R0.getTransform(), traktor::test::compareTransformEqual);
	CASE_ASSERT_COMPARE(S0.getLinearVelocity(), R0.getLinearVelocity(), traktor::test::compareVectorEqual);
	CASE_ASSERT_COMPARE(S0.getAngularVelocity(), R0.getAngularVelocity(), traktor::test::compareVectorEqual);

	CASE_ASSERT_COMPARE(S1.getTransform(), R1.getTransform(), traktor::test::compareTransformEqual);
	CASE_ASSERT_COMPARE(S1.getLinearVelocity(), R1.getLinearVelocity(), traktor::test::compareVectorEqual);
	CASE_ASSERT_COMPARE(S1.getAngularVelocity(), R1.getAngularVelocity(), traktor::test::compareVectorEqual);

	BodyState R2 = S0.interpolate(S1, 0.5_simd);
	BodyState R3 = S1.interpolate(S0, 0.5_simd);

	CASE_ASSERT_COMPARE(R2.getTransform(), R3.getTransform(), traktor::test::compareTransformEqual);
	CASE_ASSERT_COMPARE(R2.getLinearVelocity(), R3.getLinearVelocity(), traktor::test::compareVectorEqual);
	CASE_ASSERT_COMPARE(R2.getAngularVelocity(), R3.getAngularVelocity(), traktor::test::compareVectorEqual);
}

}
