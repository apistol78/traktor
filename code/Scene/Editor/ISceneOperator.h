/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

}

namespace traktor::db
{

class Database;
class Instance;

}

namespace traktor::editor
{

class IPipelineBuilder;
class IPipelineDepends;
class IPipelineSettings;

}

namespace traktor::scene
{

class SceneAsset;

/*!
 * \ingroup Scene
 */
class T_DLLCLASS ISceneOperator : public Object
{
	T_RTTI_CLASS;

public:
	enum class TransformMode
	{
		Pipeline
	};

	class T_DLLCLASS TransformContext : public Object
	{
	public:
		virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) const = 0;

		template < typename ObjectType >
		Ref< const ObjectType > getObjectReadOnly(const Guid& instanceGuid) const {
			return dynamic_type_cast< const ObjectType* >(getObjectReadOnly(instanceGuid));
		}

		virtual db::Database* getSourceDatabase() const = 0;
	};

	virtual bool create(const editor::IPipelineSettings* settings) = 0;

	virtual void destroy() = 0;

	virtual TypeInfoSet getOperatorTypes() const = 0;

	/*! Add dependencies which might get used by operator. */
	virtual void addDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const ISerializable* operatorData
	) const = 0;

	/*! Whether transform performs a geometric scene mutation. */
	virtual bool isGeometricTransform() const { return false; }

	/*! Transform scene asset. */
	virtual bool transform(
		const TransformContext& context,
		const ISerializable* operatorData,
		SceneAsset* inoutSceneAsset
	) const = 0;

	/*! */
	virtual bool build(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* operatorData,
		const db::Instance* sourceInstance,
		SceneAsset* inoutSceneAsset,
		bool rebuild
	) const = 0;
};

}
