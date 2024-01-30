/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Spray/Sequence.h"
#include "Spray/SequenceData.h"
#include "World/IEntityEventData.h"
#include "World/IEntityFactory.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.SequenceData", 0, SequenceData, ISerializable)

Ref< Sequence > SequenceData::createSequence(const world::IEntityFactory* entityFactory) const
{
	AlignedVector< Sequence::Key > keys;
	keys.resize(m_keys.size());
	for (size_t i = 0; i < m_keys.size(); ++i)
	{
		keys[i].T = m_keys[i].T;
		keys[i].event = entityFactory->createEntityEvent(nullptr, *m_keys[i].event);
	}
	return new Sequence(keys);
}

void SequenceData::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Key, MemberComposite< Key > >(L"keys", m_keys);
}

void SequenceData::Key::serialize(ISerializer& s)
{
	s >> Member< float >(L"T", T);
	s >> MemberRef< world::IEntityEventData >(L"event", event);
}

}
