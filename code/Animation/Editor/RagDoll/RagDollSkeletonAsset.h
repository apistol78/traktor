/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallSet.h"
#include "Core/Ref.h"
#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

#include <functional>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class CollisionSpecification;

}

namespace traktor::animation
{

class RagDollBone;

/*! RagDoll physics skeleton asset.
 * \ingroup Animation
 */
class T_DLLCLASS RagDollSkeletonAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
    void traverse(const std::function< void(const RagDollBone*, const RagDollBone*, const Transform& tip) >& fn) const;

    void setRoot(RagDollBone* root);

    const RagDollBone* getRoot() const { return m_root; }

    const Vector4& getRootOffset() const { return m_rootOffset; }

    const SmallSet< resource::Id< physics::CollisionSpecification > >& getCollisionGroup() const { return m_collisionGroup; }

    const SmallSet< resource::Id< physics::CollisionSpecification > >& getCollisionMask() const { return m_collisionMask; }

	virtual void serialize(ISerializer& s) override final;

private:
    Ref< RagDollBone > m_root;
    Vector4 m_rootOffset = Vector4::origo();
    SmallSet< resource::Id< physics::CollisionSpecification > > m_collisionGroup;
    SmallSet< resource::Id< physics::CollisionSpecification > > m_collisionMask;
};

}
