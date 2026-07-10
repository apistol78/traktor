/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/IPoseController.h"
#include "Core/RefArray.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class Body;
class Joint;
class ShapeDesc;

}

namespace traktor::animation
{

/*! Rag doll pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS RagDollPoseController : public IPoseController
{
	T_RTTI_CLASS;

public:
	/*! Influence of a single rag doll limb onto an animation joint. */
	struct Binding
	{
		int32_t limb = -1;				//!< Index of driving limb body.
		Scalar weight = 1.0_simd;		//!< Blend weight of the limb's contribution.
		Transform limbToJoint;			//!< Fixed transform from limb space to joint space, evaluated at bind pose.
	};

	virtual ~RagDollPoseController();

	/*! Initialize controller with a pre-built set of limbs, joints and joint bindings.
	 *
	 * \param limbs Rag doll limb bodies.
	 * \param limbShapes Collision shape of each limb body, parallel to \a limbs (used for debugging).
	 * \param joints Rag doll constraint joints.
	 * \param jointBindings Bindings from limbs to animation joints, indexed by animation joint.
	 * \param worldTransform Initial world transform of owner entity.
	 */
	bool create(
		const RefArray< physics::Body >& limbs,
		const RefArray< const physics::ShapeDesc >& limbShapes,
		const RefArray< physics::Joint >& joints,
		const AlignedVector< AlignedVector< Binding > >& jointBindings,
		const Transform& worldTransform
	);

	virtual void destroy() override final;

	virtual void reset(
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& poseTransforms) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& boneTransforms,
		AlignedVector< Transform >& outPoseTransforms
	) override final;

	/*! Apply an impulse to the rag doll, distributed across its limbs by mass.
	 *
	 * The world-space \a impulse is split among the dynamic limbs in proportion to their
	 * mass, so the net momentum change equals \a impulse and every limb receives the same
	 * velocity change (\a impulse / total mass) - equivalent to applying the impulse at the
	 * rag doll's centre of mass: a coherent whole-body push, with no spurious spin from an
	 * arbitrary application point and no single limb torn away from its joints.
	 */
	void addImpulse(const Vector4& impulse);

	/*! Apply an impulse at a world-space point, to the nearest limb.
	 *
	 * Unlike addImpulse (a uniform whole-body shove), this applies \a impulse at the world
	 * point \a at on the closest limb, so that limb gains both linear and angular velocity
	 * - a localized hit - and drags the rest of the rag doll along through the joints.
	 */
	void addImpulseAt(const Vector4& at, const Vector4& impulse);

	const RefArray< physics::Body >& getLimbs() const { return m_limbs; }

	const RefArray< const physics::ShapeDesc >& getLimbShapes() const { return m_limbShapes; }

	const RefArray< physics::Joint >& getJoints() const { return m_joints; }

private:
	RefArray< physics::Body > m_limbs;
	RefArray< const physics::ShapeDesc > m_limbShapes;
	RefArray< physics::Joint > m_joints;
	AlignedVector< AlignedVector< Binding > > m_jointBindings;
	Transform m_worldTransform = Transform::identity();
};

}
