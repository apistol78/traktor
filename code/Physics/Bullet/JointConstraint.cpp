/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Bullet/JointConstraint.h"
#include "Physics/Bullet/JointBullet.h"

namespace traktor::physics
{

JointConstraint::JointConstraint(btRigidBody& rbA)
:	btTypedConstraint(POINT2POINT_CONSTRAINT_TYPE, rbA)
,	m_jointSolver(nullptr)
{
}

JointConstraint::JointConstraint(btRigidBody& rbA, btRigidBody& rbB)
:	btTypedConstraint(POINT2POINT_CONSTRAINT_TYPE, rbA, rbB)
,	m_jointSolver(nullptr)
{
}

void JointConstraint::setJointSolver(JointSolver* jointSolver)
{
	m_jointSolver = jointSolver;
}

void JointConstraint::buildJacobian()
{
	if (m_jointSolver)
		m_jointSolver->prepare();
}

void JointConstraint::getInfo1(btConstraintInfo1* info)
{
}

void JointConstraint::getInfo2(btConstraintInfo2* info)
{
}

void JointConstraint::solveConstraintObsolete(btSolverBody& bodyA, btSolverBody& bodyB, btScalar timeStep)
{
	if (m_jointSolver)
		m_jointSolver->update(float(timeStep));
}

void JointConstraint::setParam(int num, btScalar value, int axis)
{
}

btScalar JointConstraint::getParam(int num, int axis) const
{
	return btScalar(0.0f);
}

}
