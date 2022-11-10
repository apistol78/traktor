/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/IEntityReplicator.h"

namespace traktor
{
    namespace scene
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.IEntityReplicator", IEntityReplicator, Object)

const wchar_t* IEntityReplicator::VisualMesh = L"Visual.Mesh";
const wchar_t* IEntityReplicator::CollisionMesh = L"Collision.Mesh";
const wchar_t* IEntityReplicator::CollisionShape = L"Collision.Shape";
const wchar_t* IEntityReplicator::CollisionBody = L"Collision.Body";

    }
}
