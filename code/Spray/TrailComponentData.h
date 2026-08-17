/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spray/TrailData.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::spray
{

class TrailComponent;

/*! Ribbon trail component persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS TrailComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< TrailComponent > createComponent(resource::IResourceManager* resourceManager) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const TrailData& getTrail() const { return m_trail; }

private:
	TrailData m_trail;
};

}
