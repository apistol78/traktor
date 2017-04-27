/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IPipelineCache_H
#define traktor_editor_IPipelineCache_H

#include "Core/Object.h"
#include "Core/Guid.h"
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

class IStream;
class PropertyGroup;

	namespace editor
	{

class T_DLLCLASS IPipelineCache : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(const PropertyGroup* settings) = 0;

	virtual void destroy() = 0;

	virtual Ref< IStream > get(const Guid& guid, const PipelineDependencyHash& hash) = 0;

	virtual Ref< IStream > put(const Guid& guid, const PipelineDependencyHash& hash) = 0;
};

	}
}

#endif	// traktor_editor_IPipelineCache_H
