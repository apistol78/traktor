/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Track.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.Track", Track, Object)

Track::Track(
	const Guid& entityId,
	const Guid& lookAtEntityId,
	const TransformPath& path
)
:	m_entityId(entityId)
,	m_lookAtEntityId(lookAtEntityId)
,	m_path(path)
{
}

const Guid& Track::getEntityId() const
{
	return m_entityId;
}

const Guid& Track::getLookAtEntityId() const
{
	return m_lookAtEntityId;
}

const TransformPath& Track::getPath() const
{
	return m_path;
}

TransformPath& Track::getPath()
{
	return m_path;
}

}
