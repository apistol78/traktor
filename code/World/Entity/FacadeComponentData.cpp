/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/EntityData.h"
#include "World/Entity/FacadeComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.FacadeComponentData", 0, FacadeComponentData, IEntityComponentData)

const std::wstring& FacadeComponentData::getShow() const
{
	return m_show;
}

int32_t FacadeComponentData::getOrdinal() const
{
	return 0;
}

void FacadeComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void FacadeComponentData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"show", m_show);
}

}
