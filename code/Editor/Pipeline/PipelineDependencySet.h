/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_PipelineDependencySet_H
#define traktor_editor_PipelineDependencySet_H

#include "Editor/IPipelineDependencySet.h"

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

class T_DLLCLASS PipelineDependencySet : public IPipelineDependencySet
{
	T_RTTI_CLASS;

public:
	virtual uint32_t add(PipelineDependency* dependency) T_OVERRIDE T_FINAL;

	virtual uint32_t add(const Guid& dependencyGuid, PipelineDependency* dependency) T_OVERRIDE T_FINAL;

	virtual PipelineDependency* get(uint32_t index) const T_OVERRIDE T_FINAL;

	virtual uint32_t get(const Guid& dependencyGuid) const T_OVERRIDE T_FINAL;

	virtual uint32_t size() const T_OVERRIDE T_FINAL;

	virtual void dump(OutputStream& os) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	RefArray< PipelineDependency > m_dependencies;
	std::map< Guid, uint32_t > m_indices;
};

	}
}

#endif	// traktor_editor_PipelineDependencySet_H
