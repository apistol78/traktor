#ifndef traktor_db_EvtInstance_H
#define traktor_db_EvtInstance_H

#include "Core/Guid.h"
#include "Database/IEvent.h"

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

class T_DLLCLASS EvtInstance : public IEvent
{
	T_RTTI_CLASS;

public:
	EvtInstance(const Guid& instanceGuid);

	const Guid& getInstanceGuid() const;

	virtual void serialize(ISerializer& s);

private:
	Guid m_instanceGuid;
};

	}
}

#endif	// traktor_db_EvtInstance_H
