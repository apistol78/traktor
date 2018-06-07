/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IPipelineBuilder_H
#define traktor_editor_IPipelineBuilder_H

#include "Core/RefArray.h"
#include "Editor/IPipelineCommon.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace editor
	{

class IPipelineDependencySet;
class PipelineDependency;

/*! \brief Pipeline builder interface.
 * \ingroup Editor
 */
class T_DLLCLASS IPipelineBuilder : public IPipelineCommon
{
	T_RTTI_CLASS;

public:
	enum BuildResult
	{
		BrSucceeded = 0,
		BrSucceededWithWarnings = 1,
		BrFailed = 2
	};

	struct IListener
	{
		virtual ~IListener() {}

		virtual void beginBuild(
			int32_t core,
			int32_t index,
			int32_t count,
			const PipelineDependency* dependency
		) = 0;

		virtual void endBuild(
			int32_t core,
			int32_t index,
			int32_t count,
			const PipelineDependency* dependency,
			BuildResult result
		) = 0;
	};

	virtual bool build(const IPipelineDependencySet* dependencySet, bool rebuild) = 0;

	virtual Ref< ISerializable > buildOutput(const ISerializable* sourceAsset) = 0;

	virtual bool buildOutput(const ISerializable* sourceAsset, const std::wstring& outputPath, const Guid& outputGuid, const Object* buildParams = 0) = 0;

	virtual Ref< ISerializable > getBuildProduct(const ISerializable* sourceAsset) = 0;

	virtual Ref< db::Instance > createOutputInstance(const std::wstring& instancePath, const Guid& instanceGuid) = 0;

	virtual Ref< db::Database > getOutputDatabase() const = 0;
};

	}
}

#endif	// traktor_editor_IPipelineBuilder_H
