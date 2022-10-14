#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;
class Guid;

}

namespace traktor::editor
{

/*! Pipeline database instance object read-only cache.
 * \ingroup Editor
 */
class T_DLLCLASS IPipelineInstanceCache : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< const ISerializable > getObjectReadOnly(const Guid& instanceGuid) = 0;

	virtual void flush(const Guid& instanceGuid) = 0;
};

}
