/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace world
	{

class EntityData;

	}

	namespace scene
	{

class EntityClipboardData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addEntityData(world::EntityData* entityData);

	const RefArray< world::EntityData >& getEntityData() const;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< world::EntityData > m_entityData;
};

	}
}

