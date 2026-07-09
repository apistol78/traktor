/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector4.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

/*! RagDoll physics bone.
 * \ingroup Animation
 *
 * A physics bone is defined in relation to the end tip of it's
 * parent bone.
 */
class T_DLLCLASS RagDollBone : public ISerializable
{
	T_RTTI_CLASS;

public:
    struct Influence
    {
        std::wstring joint;
        float weight = 1.0f;

        void serialize(ISerializer& s);
    };

    /*! Description of the physics constraint connecting this bone to its parent. */
    struct Constraint
    {
        enum class Type
        {
            Free = 0,   //!< Point constraint only; rotation unconstrained (ball joint).
            Cone = 1,   //!< Swing (cone) and twist limited (cone-twist joint).
            Hinge = 2   //!< Single axis with angular limits (hinge joint).
        };

        //! Bone-local axis (with sign) that a hinge rotates about; the sign flips the bend direction.
        enum class Axis
        {
            X = 0,
            Y = 1,
            Z = 2,
            NegX = 3,
            NegY = 4,
            NegZ = 5
        };

        Type type = Type::Free;
        float swingAngle = 0.0f;    //!< Cone: swing half-cone angle (radians).
        float twistAngle = 0.0f;    //!< Cone: twist limit angle (radians).
        float minAngle = 0.0f;      //!< Hinge: minimum angle (radians).
        float maxAngle = 0.0f;      //!< Hinge: maximum angle (radians).
        Axis hingeAxis = Axis::X;   //!< Hinge: bone-local axis to rotate about.

        void serialize(ISerializer& s);
    };

    const Vector4& getOffset() const { return m_offset; }

    const Quaternion& getRotation() const { return m_rotation; }

    float getLength() const { return m_length; }

    float getRadius() const { return m_radius; }

    float getLinearDamping() const { return m_linearDamping; }

    float getAngularDamping() const { return m_angularDamping; }

    const AlignedVector< Influence >& getInfluences() const { return m_influences; }

    void addChild(RagDollBone* child) { m_children.push_back(child); }

    void removeChild(RagDollBone* child) { m_children.remove(child); }

    const RefArray< RagDollBone >& getChildren() const { return m_children; }

    const Constraint& getConstraint() const { return m_constraint; }

	virtual void serialize(ISerializer& s) override final;

private:
    Vector4 m_offset = Vector4::zero();
    Quaternion m_rotation = Quaternion::identity();
    float m_length = 1.0f;
    float m_radius = 0.5f;
    float m_linearDamping = 0.0f;
    float m_angularDamping = 0.0f;
    AlignedVector< Influence > m_influences;
    RefArray< RagDollBone > m_children;
    Constraint m_constraint;
};

}
