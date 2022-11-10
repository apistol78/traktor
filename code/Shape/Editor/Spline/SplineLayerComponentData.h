/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class TransformPath;

	namespace db
	{
	
class Database;

	}

	namespace model
	{
	
class Model;
class ModelCache;
	
	}

	namespace shape
	{

class SplineLayerComponent;

class T_DLLCLASS SplineLayerComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual Ref< SplineLayerComponent > createComponent(db::Database* database) const = 0;

	virtual Ref< model::Model > createModel(db::Database* database, model::ModelCache* modelCache, const std::wstring& assetPath, const TransformPath& path) const = 0;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override;
};

	}
}
