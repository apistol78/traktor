/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_resource_ExplicitResourceHandle_H
#define traktor_resource_ExplicitResourceHandle_H

#include "Resource/ResourceHandle.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! \brief Explicit resource handle.
 * \ingroup Resource
 */
class T_DLLCLASS ExplicitResourceHandle : public ResourceHandle
{
	T_RTTI_CLASS;

public:
	ExplicitResourceHandle(Object* object);
};

	}
}

#endif	// traktor_resource_ExplicitResourceHandle_H
