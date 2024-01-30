/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class Entity;
struct UpdateParams;

/*! World container.
 * 
 * The world is a container of all entities representing a world.
 * 
 * \ingroup World
 */
class T_DLLCLASS World : public Object
{
	T_RTTI_CLASS;

public:
	void destroy();

	void addEntity(Entity* entity);

	void removeEntity(Entity* entity);

	bool haveEntity(const Entity* entity) const;

	Entity* getEntity(const std::wstring_view& name, int32_t index) const;

	RefArray< Entity > getEntities(const std::wstring_view& name) const;

	void update(const UpdateParams& update);

	const RefArray< Entity >& getEntities() const { return m_entities; }

private:
	RefArray< Entity > m_entities;
};

}
