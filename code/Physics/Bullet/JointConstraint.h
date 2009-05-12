#ifndef traktor_physics_JointConstraint_H
#define traktor_physics_JointConstraint_H

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
	JointConstraint(btRigidBody& rbA, btRigidBody& rbB);

	void setJointSolver(JointSolver* jointSolver);

	virtual void buildJacobian();

	virtual void getInfo1(btConstraintInfo1* info);

	virtual void getInfo2(btConstraintInfo2* info);

	virtual	void solveConstraintObsolete(btSolverBody& bodyA, btSolverBody& bodyB, btScalar timeStep);

private:
	JointSolver* m_jointSolver;
};

	}
}

#endif	// traktor_physics_JointConstraint_H
