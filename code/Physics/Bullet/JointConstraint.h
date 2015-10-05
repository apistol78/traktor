#ifndef traktor_physics_JointConstraint_H
#define traktor_physics_JointConstraint_H

#include "Core/Config.h"
#include <BulletDynamics/ConstraintSolver/btTypedConstraint.h>

namespace traktor
{
	namespace physics
	{

struct JointSolver;

/*! \brief Bullet constraint type to enable custom joints.
 * \ingroup Bullet
 */
class JointConstraint : public btTypedConstraint
{
public:
	JointConstraint(btRigidBody& rbA);

	JointConstraint(btRigidBody& rbA, btRigidBody& rbB);

	void setJointSolver(JointSolver* jointSolver);

	virtual void buildJacobian() T_OVERRIDE T_FINAL;

	virtual void getInfo1(btConstraintInfo1* info) T_OVERRIDE T_FINAL;

	virtual void getInfo2(btConstraintInfo2* info) T_OVERRIDE T_FINAL;

	virtual	void solveConstraintObsolete(btSolverBody& bodyA, btSolverBody& bodyB, btScalar timeStep) T_OVERRIDE T_FINAL;

	virtual	void setParam(int num, btScalar value, int axis) T_OVERRIDE T_FINAL;

	virtual	btScalar getParam(int num, int axis) const T_OVERRIDE T_FINAL;

private:
	JointSolver* m_jointSolver;
};

	}
}

#endif	// traktor_physics_JointConstraint_H
