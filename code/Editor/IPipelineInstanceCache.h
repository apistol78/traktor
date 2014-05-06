#ifndef traktor_editor_IPipelineInstanceCache_H
#define traktor_editor_IPipelineInstanceCache_H

#include "Core/Object.h"

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
	virtual Ref< ISerializable > getObjectReadOnly(const Guid& instanceGuid) = 0;

	virtual void flush(const Guid& instanceGuid) = 0;
};

	}
}

#endif	// traktor_editor_IPipelineInstanceCache_H
