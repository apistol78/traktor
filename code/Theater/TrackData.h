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
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"
#include "Theater/PropertyPath.h"

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
 *
 * A track animate a single entity through two paths; one for the transform of
 * the entity itself and one for the properties of its components.
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

	const AlignedVector< EventKey >& getEvents() const;

	AlignedVector< EventKey >& getEvents();

	/*! Get path animating the transform of the entity; null if it isn't animated. */
	PropertyPath* getTransform() const { return m_transform; }

	void setTransform(PropertyPath* transform) { m_transform = transform; }

	/*! Get path animating properties of the entity's components; null if none are. */
	PropertyPath* getProperties() const { return m_properties; }

	void setProperties(PropertyPath* properties) { m_properties = properties; }

	/*! Get a copy of this track, with every path stripped. */
	Ref< TrackData > stripped() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_entityId;
	Guid m_lookAtEntityId;
	AlignedVector< EventKey > m_events;
	Ref< PropertyPath > m_transform;
	Ref< PropertyPath > m_properties;
};

}
