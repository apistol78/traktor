#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class ISerializable;
class Guid;

	namespace editor
	{

/*! Pipeline database instance object read-only cache.
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

