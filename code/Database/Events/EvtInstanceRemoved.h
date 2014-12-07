#ifndef traktor_db_EvtInstanceRemoved_H
#define traktor_db_EvtInstanceRemoved_H

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

/*! \brief Instance removed event.
 * \ingroup Database
 */
class T_DLLCLASS EvtInstanceRemoved : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceRemoved(const Guid& instanceGuid = Guid());
};

	}
}

#endif	// traktor_db_EvtInstanceRemoved_H
