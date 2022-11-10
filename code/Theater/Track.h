/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Math/TransformPath.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;

	}

	namespace theater
	{

/*! Track
 * \ingroup Theater
 */
class T_DLLCLASS Track : public Object
{
	T_RTTI_CLASS;

public:
	Track(
		world::Entity* entity,
		world::Entity* lookAtEntity,
		const TransformPath& path
	);

	void setEntity(world::Entity* entity);

	world::Entity* getEntity() const;

	void setLookAtEntity(world::Entity* entity);

	world::Entity* getLookAtEntity() const;

	const TransformPath& getPath() const;

	TransformPath& getPath();

private:
	world::Entity* m_entity;
	world::Entity* m_lookAtEntity;
	TransformPath m_path;
};

	}
}

