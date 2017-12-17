/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IPipelineDb_H
#define traktor_editor_IPipelineDb_H

#include "Core/Guid.h"
#include "Core/Io/Path.h"
#include "Editor/PipelineTypes.h"

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

/*! \brief Pipeline database.
 * \ingroup Editor
 *
 * The pipeline db keeps records of builds
 * used by the pipeline to check if an asset
 * needs to be rebuilt.
 *
 * It also keeps information about
 * the current status of the products
 * which can be aggregated to produce
 * statistics of the data.
 */
class T_DLLCLASS IPipelineDb : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool open(const std::wstring& connectionString) = 0;

	virtual void close() = 0;

	virtual void beginTransaction() = 0;

	virtual void endTransaction() = 0;

	virtual void setDependency(const Guid& guid, const PipelineDependencyHash& hash) = 0;

	virtual bool getDependency(const Guid& guid, PipelineDependencyHash& outHash) const = 0;

	virtual void setFile(const Path& path, const PipelineFileHash& file) = 0;

	virtual bool getFile(const Path& path, PipelineFileHash& outFile) = 0;
};

	}
}

#endif	// traktor_editor_IPipelineDb_H
