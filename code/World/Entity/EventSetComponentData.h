/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class EventSetComponent;
class IEntityBuilder;
class IEntityEventData;

/*!
 * \ingroup World
 */
class T_DLLCLASS EventSetComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< EventSetComponent > createComponent(const IEntityBuilder* entityBuilder) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class WorldEntityPipeline;

	SmallMap< std::wstring, Ref< IEntityEventData > > m_eventData;
};

}

