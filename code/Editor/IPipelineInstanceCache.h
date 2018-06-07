/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_IPipelineInstanceCache_H
#define traktor_editor_IPipelineInstanceCache_H

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class ISerializable;
class Guid;

	namespace editor
	{

/*! \brief Pipeline database instance object read-only cache.
 * \ingroup Editor
 */
class IPipelineInstanceCache : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) = 0;

	virtual void flush(const Guid& instanceGuid) = 0;
};

	}
}

#endif	// traktor_editor_IPipelineInstanceCache_H
