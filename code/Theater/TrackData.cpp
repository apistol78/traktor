/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Theater/TrackData.h"
#include "World/EntityData.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TrackData", 0, TrackData, ISerializable)

void TrackData::setEntityId(const Guid& entityId)
{
	m_entityId = entityId;
}

const Guid& TrackData::getEntityId() const
{
	return m_entityId;
}

void TrackData::setLookAtEntityId(const Guid& entityId)
{
	m_lookAtEntityId = entityId;
}

const Guid& TrackData::getLookAtEntityId() const
{
	return m_lookAtEntityId;
}

void TrackData::setPath(const TransformPath& path)
{
	m_path = path;
}

const TransformPath& TrackData::getPath() const
{
	return m_path;
}

TransformPath& TrackData::getPath()
{
	return m_path;
}

void TrackData::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"entityId", m_entityId, AttributePrivate());
	s >> Member< Guid >(L"lookAtEntityId", m_lookAtEntityId, AttributePrivate());
	s >> MemberComposite< TransformPath >(L"path", m_path);
}

}
