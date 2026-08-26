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
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Theater/PropertyPath.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeDispatch;

}

namespace traktor::world
{

class IEntityEvent;

}

namespace traktor::theater
{

/*! Track
 * \ingroup Theater
 */
class T_DLLCLASS Track : public Object
{
	T_RTTI_CLASS;

public:
	/*! Event issued by the track's entity at a given time. */
	struct EventKey
	{
		float T = 0.0f;
		Ref< const world::IEntityEvent > event;
	};

	/*! Property, of the track's entity or one of its components, animated by the track.
	 *
	 * Accessors are resolved once from the runtime class of the object; the type
	 * of the value is resolved from the first object the property is applied to.
	 */
	struct Property
	{
		Ref< const PropertyPath > path;
		int32_t index = -1;

		/*! Type of the component owning the property; null if the entity itself. */
		const TypeInfo* componentType = nullptr;
		const IRuntimeDispatch* getter = nullptr;
		const IRuntimeDispatch* setter = nullptr;
		mutable PropertyPath::ValueType valueType = PropertyPath::ValueType::Invalid;
		mutable bool valueTypeResolved = false;
	};

	explicit Track(
		const Guid& entityId,
		const Guid& lookAtEntityId,
		const AlignedVector< EventKey >& events,
		const AlignedVector< Property >& properties
	);

	const Guid& getEntityId() const;

	const Guid& getLookAtEntityId() const;

	const AlignedVector< EventKey >& getEvents() const;

	const AlignedVector< Property >& getProperties() const;

	/*! Get property animating the transform of the entity; null if it isn't animated. */
	const Property* getTransform() const;

private:
	Guid m_entityId;
	Guid m_lookAtEntityId;
	AlignedVector< EventKey > m_events;
	AlignedVector< Property > m_properties;
	int32_t m_transform = -1;
};

}
