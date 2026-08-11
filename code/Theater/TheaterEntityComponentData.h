/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityBuilder;

}

namespace traktor::theater
{

class ActData;
class TheaterEntityComponent;

/*! Theater entity component data.
 *
 * Keyframed animation of child entities of the owner entity's group;
 * all tracks are relative the owner's transform.
 *
 * \ingroup Theater
 */
class T_DLLCLASS TheaterEntityComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< TheaterEntityComponent > createComponent(const world::IEntityBuilder* entityBuilder) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const RefArray< ActData >& getActs() const { return m_acts; }

	RefArray< ActData >& getActs() { return m_acts; }

private:
	RefArray< ActData > m_acts;
};

}
