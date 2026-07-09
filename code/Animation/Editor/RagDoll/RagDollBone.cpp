/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/RagDoll/RagDollBone.h"

#include "Core/Math/Const.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RagDollBone", 4, RagDollBone, ISerializable)

void RagDollBone::serialize(ISerializer& s)
{
    if (s.getVersion< RagDollBone >() >= 1)
        s >> Member< Vector4 >(L"offset", m_offset, AttributePoint());

    s >> Member< Quaternion >(L"rotation", m_rotation);
    s >> Member< float >(L"length", m_length);
    s >> Member< float >(L"radius", m_radius);

    if (s.getVersion< RagDollBone >() >= 4)
    {
        s >> Member< float >(L"linearDamping", m_linearDamping, AttributeRange(0.0f, 1.0f));
        s >> Member< float >(L"angularDamping", m_angularDamping, AttributeRange(0.0f, 1.0f));
    }

    s >> MemberAlignedVector< Influence, MemberComposite< Influence > >(L"influences", m_influences);
    s >> MemberRefArray< RagDollBone >(L"children", m_children, AttributePrivate());

    if (s.getVersion< RagDollBone >() >= 2)
        s >> MemberComposite< Constraint >(L"constraint", m_constraint);
}

void RagDollBone::Influence::serialize(ISerializer& s)
{
    s >> Member< std::wstring >(L"joint", joint);
    s >> Member< float >(L"weight", weight);
}

void RagDollBone::Constraint::serialize(ISerializer& s)
{
    const MemberEnum< Type >::Key c_Type_Keys[] = {
        { L"Free", Type::Free },
        { L"Cone", Type::Cone },
        { L"Hinge", Type::Hinge },
        { 0 }
    };

    const MemberEnum< Axis >::Key c_Axis_Keys[] = {
        { L"X", Axis::X },
        { L"Y", Axis::Y },
        { L"Z", Axis::Z },
        { L"-X", Axis::NegX },
        { L"-Y", Axis::NegY },
        { L"-Z", Axis::NegZ },
        { 0 }
    };

    s >> MemberEnum< Type >(L"type", type, c_Type_Keys);
    s >> Member< float >(L"swingAngle", swingAngle, AttributeUnit(UnitType::Radians) | AttributeRange(0.0f, PI));
    s >> Member< float >(L"twistAngle", twistAngle, AttributeUnit(UnitType::Radians) | AttributeRange(0.0f, PI));
    s >> Member< float >(L"minAngle", minAngle, AttributeUnit(UnitType::Radians) | AttributeRange(-PI, PI));
    s >> Member< float >(L"maxAngle", maxAngle, AttributeUnit(UnitType::Radians) | AttributeRange(-PI, PI));

    if (s.getVersion< RagDollBone >() >= 3)
        s >> MemberEnum< Axis >(L"hingeAxis", hingeAxis, c_Axis_Keys);
}

}
