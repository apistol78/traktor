/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ai
{

class NavMesh;

/*! Navigation mesh entity data.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	const resource::Id< NavMesh >& get() const { return m_navMesh; }

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	resource::Id< NavMesh > m_navMesh;
};

}
