/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/Bullet/JointConstraint.h"
#include "Physics/Bullet/JointBullet.h"

namespace traktor
{
	namespace physics
	{

JointConstraint::JointConstraint(btRigidBody& rbA)
:	btTypedConstraint(POINT2POINT_CONSTRAINT_TYPE, rbA)
,	m_jointSolver(0)
{
}

JointConstraint::JointConstraint(btRigidBody& rbA, btRigidBody& rbB)
:	btTypedConstraint(POINT2POINT_CONSTRAINT_TYPE, rbA, rbB)
,	m_jointSolver(0)
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
}
