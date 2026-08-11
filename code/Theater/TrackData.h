/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/TransformPath.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityEventData;

}

namespace traktor::theater
{

/*! Track data.
 * \ingroup Theater
 */
class T_DLLCLASS TrackData : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Event issued by the track's entity at a given time. */
	struct EventKey
	{
		float T = 0.0f;
		Ref< world::IEntityEventData > event;

		void serialize(ISerializer& s);
	};

	void setEntityId(const Guid& entityId);

	const Guid& getEntityId() const;

	void setLookAtEntityId(const Guid& entityId);

	const Guid& getLookAtEntityId() const;

	void setPath(const TransformPath& path);

	const TransformPath& getPath() const;

	TransformPath& getPath();

	const AlignedVector< EventKey >& getEvents() const;

	AlignedVector< EventKey >& getEvents();

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_entityId;
	Guid m_lookAtEntityId;
	TransformPath m_path;
	AlignedVector< EventKey > m_events;
};

}
