/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_DefaultPipeline_H
#define traktor_editor_DefaultPipeline_H

#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Default pipeline.
 * \ingroup Editor
 *
 * This pipeline is used for every source instance
 * which hasn't a dedicated pipeline.
 *
 * It copies the source instance to the
 * output database without any modification.
 */
class T_DLLCLASS DefaultPipeline : public IPipeline
{
	T_RTTI_CLASS;

public:
	virtual bool create(const IPipelineSettings* settings) T_OVERRIDE;

	virtual void destroy() T_OVERRIDE;

	virtual TypeInfoSet getAssetTypes() const T_OVERRIDE;

	virtual bool buildDependencies(
		IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const T_OVERRIDE;

	virtual bool buildOutput(
		IPipelineBuilder* pipelineBuilder,
		const IPipelineDependencySet* dependencySet,
		const PipelineDependency* dependency,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const T_OVERRIDE;

	virtual Ref< ISerializable > buildOutput(
		IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const T_OVERRIDE;
};

	}
}

#endif	// traktor_editor_DefaultPipeline_H
