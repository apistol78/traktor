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
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
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

class IEntityEvent;
class IEntityEventInstance;

class T_DLLCLASS EventSetComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const UpdateParams& update) override final;

	IEntityEventInstance* raise(const std::wstring& name, const Transform& offset = Transform::identity());

	const IEntityEvent* getEvent(const std::wstring& name) const;

private:
	friend class EventSetComponentData;

	Entity* m_owner = nullptr;
	SmallMap< std::wstring, Ref< const IEntityEvent > > m_events;
};

}
