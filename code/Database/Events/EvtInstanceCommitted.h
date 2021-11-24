#pragma once

#include "Database/Events/EvtInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! Instance committed event.
 * \ingroup Database
 */
class T_DLLCLASS EvtInstanceCommitted : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceCommitted() = default;

	explicit EvtInstanceCommitted(const Guid& instanceGuid);
};

	}
}

