/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/RagDoll/RagDollSkeletonAsset.h"

#include "Animation/Editor/RagDoll/RagDollBone.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallSet.h"
#include "Physics/CollisionSpecification.h"
#include "Resource/Member.h"

namespace traktor::animation
{
    namespace
    {

void traverseBone(
    const RagDollBone* parent,
    const RagDollBone* bone,
    const Transform& parentTip,
    const std::function< void(const RagDollBone*, const RagDollBone*, const Transform& tip) >& fn
)
{
    // Invoke callback with the tip of the parent bone; this is the
    // frame from which the current bone is defined.
    fn(parent, bone, parentTip);

    // Calculate tip of the current bone; the bone is offset and oriented relative
    // to the parent's tip and extends along its local Z axis by its length.
    const Transform tip =
        parentTip *
        Transform(bone->getOffset(), bone->getRotation()) *
        Transform(Vector4(0.0f, 0.0f, bone->getLength(), 0.0f));

    // Recurse into children, defined relative to this bone's tip.
    for (auto child : bone->getChildren())
        traverseBone(bone, child, tip, fn);
}

    }

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.RagDollSkeletonAsset", 1, RagDollSkeletonAsset, ISerializable)

void RagDollSkeletonAsset::setRoot(RagDollBone* root)
{
    m_root = root;
}

void RagDollSkeletonAsset::traverse(const std::function< void(const RagDollBone*, const RagDollBone*, const Transform& tip) >& fn) const
{
    if (!m_root)
        return;

    // The root bone has no parent; it is anchored at the root offset.
    traverseBone(nullptr, m_root, Transform(m_rootOffset), fn);
}

void RagDollSkeletonAsset::serialize(ISerializer& s)
{
    s >> MemberRef< RagDollBone >(L"root", m_root);
    s >> Member< Vector4 >(L"rootOffset", m_rootOffset, AttributePoint());

    if (s.getVersion< RagDollSkeletonAsset >() >= 1)
    {
        s >> MemberSmallSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionGroup", m_collisionGroup);
        s >> MemberSmallSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionMask", m_collisionMask);
    }
}

}
