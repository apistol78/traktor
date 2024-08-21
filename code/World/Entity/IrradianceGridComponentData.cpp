/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"
#include "World/IrradianceGrid.h"
#include "World/Entity/IrradianceGridComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.IrradianceGridComponentData", 0, IrradianceGridComponentData, IWorldComponentData)

IrradianceGridComponentData::IrradianceGridComponentData(const resource::Id< IrradianceGrid >& irradianceGrid)
:	m_irradianceGrid(irradianceGrid)
{
}

void IrradianceGridComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< IrradianceGrid >(L"irradianceGrid", m_irradianceGrid);
}

}
