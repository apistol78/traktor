/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/RagDoll/RagDollSkeleton.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/JointDesc.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RagDollSkeleton", 0, RagDollSkeleton, ISerializable)

void RagDollSkeleton::serialize(ISerializer& s)
{
    s >> MemberAlignedVector< Limb, MemberComposite< Limb > >(L"limbs", m_limbs);
    s >> MemberAlignedVector< Joint, MemberComposite< Joint > >(L"joints", m_joints);
}

void RagDollSkeleton::Influence::serialize(ISerializer& s)
{
    s >> Member< std::wstring >(L"joint", joint);
    s >> Member< float >(L"weight", weight);
}

void RagDollSkeleton::Limb::serialize(ISerializer& s)
{
    s >> MemberComposite< Transform >(L"transform", transform);
    s >> MemberRef< physics::DynamicBodyDesc >(L"body", body);
    s >> MemberAlignedVector< Influence, MemberComposite< Influence > >(L"influences", influences);
}

void RagDollSkeleton::Joint::serialize(ISerializer& s)
{
    s >> MemberRef< physics::JointDesc >(L"joint", joint);
    s >> Member< int32_t >(L"body1", body1);
    s >> Member< int32_t >(L"body2", body2);
}

}
