/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*!
 * \ingroup World
 */
class T_DLLCLASS EditorAttributesComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	bool visible = true;	//!< If entity should be visible, only affects editor not runtime.
	bool locked = false;	//!< If entity is locked being edited.
	bool include = true;	//!< If entity is included in build.
	bool dynamic = true;	//!< If entity is dynamic, cannot be part of any baking process etc.
};

}
