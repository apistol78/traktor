/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Io/Path.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{
	
class Database;

}

namespace traktor::shape
{

class SplineLayerComponent;

class T_DLLCLASS SplineLayerComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual Ref< SplineLayerComponent > createComponent(db::Database* database, const Path& modelCachePath, const std::wstring& assetPath) const = 0;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override;
};

}
