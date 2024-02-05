/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/RefArray.h"
#include "World/IEntityComponent.h"

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

/*! Group component.
 * \ingroup World
 */
class T_DLLCLASS GroupComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void update(const UpdateParams& update) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual void setState(const EntityState& state, const EntityState& mask) override final;

	virtual Aabb3 getBoundingBox() const override final;

	void addEntity(Entity* entity);

	void removeEntity(Entity* entity);

	void removeAllEntities();

	const RefArray< Entity >& getEntities() const;

	Entity* getEntity(const std::wstring& name, int32_t index) const;

	RefArray< Entity > getEntities(const std::wstring& name) const;

private:
	Entity* m_owner = nullptr;
	Transform m_transform;
	RefArray< Entity > m_entities;
};

}
