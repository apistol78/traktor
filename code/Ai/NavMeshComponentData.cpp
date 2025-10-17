/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMesh.h"
#include "Ai/NavMeshComponentData.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshComponentData", 0, NavMeshComponentData, world::IWorldComponentData)

void NavMeshComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< NavMesh >(L"navMesh", m_navMesh);
}

}
