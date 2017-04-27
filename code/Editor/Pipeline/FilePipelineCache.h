/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_FilePipelineCache_H
#define traktor_editor_FilePipelineCache_H

#include "Editor/IPipelineCache.h"

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

class T_DLLCLASS FilePipelineCache : public IPipelineCache
{
	T_RTTI_CLASS;

public:
	FilePipelineCache();

	virtual bool create(const PropertyGroup* settings) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual Ref< IStream > get(const Guid& guid, const PipelineDependencyHash& hash) T_OVERRIDE T_FINAL;

	virtual Ref< IStream > put(const Guid& guid, const PipelineDependencyHash& hash) T_OVERRIDE T_FINAL;

private:
	bool m_accessRead;
	bool m_accessWrite;
	std::wstring m_path;
};

	}
}

#endif	// traktor_editor_FilePipelineCache_H
