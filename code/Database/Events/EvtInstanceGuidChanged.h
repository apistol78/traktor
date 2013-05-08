#ifndef traktor_db_EvtInstanceGuidChanged_H
#define traktor_db_EvtInstanceGuidChanged_H

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

class T_DLLCLASS EvtInstanceGuidChanged : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceGuidChanged();

	EvtInstanceGuidChanged(const Guid& instanceGuid, const Guid& instancePreviousGuid);

	const Guid& getInstancePreviousGuid() const;

	virtual void serialize(ISerializer& s);

private:
	Guid m_instancePreviousGuid;
};

	}
}

#endif	// traktor_db_EvtInstanceGuidChanged_H
