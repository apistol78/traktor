/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include <BulletDynamics/ConstraintSolver/btTypedConstraint.h>

namespace traktor
{
	namespace physics
	{

struct JointSolver;

/*! Bullet constraint type to enable custom joints.
 * \ingroup Bullet
 */
class JointConstraint : public btTypedConstraint
{
public:
	JointConstraint(btRigidBody& rbA);

	JointConstraint(btRigidBody& rbA, btRigidBody& rbB);

	void setJointSolver(JointSolver* jointSolver);

	virtual void buildJacobian() override final;

	virtual void getInfo1(btConstraintInfo1* info) override final;

	virtual void getInfo2(btConstraintInfo2* info) override final;

	virtual	void solveConstraintObsolete(btSolverBody& bodyA, btSolverBody& bodyB, btScalar timeStep) override final;

	virtual	void setParam(int num, btScalar value, int axis) override final;

	virtual	btScalar getParam(int num, int axis) const override final;

private:
	JointSolver* m_jointSolver;
};

	}
}

