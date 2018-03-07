/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_MeshPipeline_H
#define traktor_mesh_MeshPipeline_H

#include "Core/Thread/Semaphore.h"
#include "Editor/IPipeline.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace render
	{

class IProgramCompiler;

	}

	namespace mesh
	{

class T_DLLCLASS MeshPipeline : public editor::IPipeline
{
	T_RTTI_CLASS;

public:
	MeshPipeline();

	virtual bool create(const editor::IPipelineSettings* settings) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual TypeInfoSet getAssetTypes() const T_OVERRIDE T_FINAL;

	virtual bool buildDependencies(
		editor::IPipelineDepends* pipelineDepends,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		const std::wstring& outputPath,
		const Guid& outputGuid
	) const T_OVERRIDE T_FINAL;

	virtual bool buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const editor::IPipelineDependencySet* dependencySet,
		const editor::PipelineDependency* dependency,
		const db::Instance* sourceInstance,
		const ISerializable* sourceAsset,
		uint32_t sourceAssetHash,
		const std::wstring& outputPath,
		const Guid& outputGuid,
		const Object* buildParams,
		uint32_t reason
	) const T_OVERRIDE T_FINAL;

	virtual Ref< ISerializable > buildOutput(
		editor::IPipelineBuilder* pipelineBuilder,
		const ISerializable* sourceAsset
	) const T_OVERRIDE T_FINAL;

private:
	std::wstring m_assetPath;
	bool m_promoteHalf;
	bool m_enableCustomShaders;
	bool m_enableCustomTemplates;
	bool m_enableBakeOcclusion;
	std::set< std::wstring > m_includeOnlyTechniques;
	std::wstring m_programCompilerTypeName;
	bool m_editor;
	mutable Semaphore m_programCompilerLock;
	mutable Ref< render::IProgramCompiler > m_programCompiler;

	render::IProgramCompiler* getProgramCompiler() const;
};

	}
}

#endif	// traktor_mesh_MeshPipeline_H
