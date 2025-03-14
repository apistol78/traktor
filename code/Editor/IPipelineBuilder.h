/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Editor/IPipelineCommon.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::db
{

class Instance;

}

namespace traktor::editor
{

class DataAccessCache;
class PipelineDependencySet;
class PipelineDependency;
class PipelineProfiler;

/*! Pipeline builder interface.
 * \ingroup Editor
 */
class T_DLLCLASS IPipelineBuilder : public IPipelineCommon
{
	T_RTTI_CLASS;

public:
	enum class BuildResult
	{
		Succeeded = 0,
		SucceededWithWarnings = 1,
		Failed = 2
	};

	struct IListener
	{
		virtual ~IListener() {}

		virtual void beginBuild(
			int32_t index,
			int32_t count,
			const PipelineDependency* dependency
		) = 0;

		virtual void endBuild(
			int32_t index,
			int32_t count,
			const PipelineDependency* dependency,
			BuildResult result
		) = 0;
	};

	//@name External interface, should be called from pipeline hosts.
	//@{

	/*! Build all instances in dependency set. */
	virtual bool build(const PipelineDependencySet* dependencySet, bool rebuild) = 0;

	//@}

	//@name Build interface, should only be called from other pipelines during build.
	//@{

	/*! Build output instance (product) from source asset.
	 *
	 * \param sourceInstance Database instance which contain sourceAsset, passed to IPipeline::buildOutput directly.
	 * \param sourceAsset Source asset.
	 * \param buildParams Optional build parameters.
	 * \return Output instance.
	 */
	virtual Ref< ISerializable > buildProduct(const db::Instance* sourceInstance, const ISerializable* sourceAsset, const Object* buildParams = nullptr) = 0;

	/*! Fake build ad-hoc output, only register ad-hoc build. */
	virtual bool buildAdHocOutput(
		const Guid& outputGuid
	) = 0;

	/*! Build ad-hoc output instance from source asset. */
	virtual bool buildAdHocOutput(
		const ISerializable* sourceAsset,
		const Guid& outputGuid,
		const Object* buildParams = nullptr
	) = 0;

	/*! Build ad-hoc output instance from source asset. */
	virtual bool buildAdHocOutput(
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams = nullptr
	) = 0;

	/*! Calculate hash of source asset including it's dependencies. */
	virtual uint32_t calculateInclusiveHash(const ISerializable* sourceAsset) const = 0;

	/*! Get build product of source asset. */
	virtual Ref< ISerializable > getBuildProduct(const ISerializable* sourceAsset) = 0;

	/*! Create output database instance. */
	virtual Ref< db::Instance > createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid) = 0;

	/*! Get reference to output database. */
	virtual db::Database* getOutputDatabase() const = 0;

	/*! Get data access cache. */
	virtual DataAccessCache* getDataAccessCache() const = 0;

	/*! Get profiler. */
	virtual PipelineProfiler* getProfiler() const = 0;

	//@}
};

}
