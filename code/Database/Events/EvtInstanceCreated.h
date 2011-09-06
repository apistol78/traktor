#ifndef traktor_db_EvtInstanceCreated_H
#define traktor_db_EvtInstanceCreated_H

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

class T_DLLCLASS EvtInstanceCreated : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceCreated(const Guid& instanceGuid = Guid());
};

	}
}

#endif	// traktor_db_EvtInstanceCreated_H
