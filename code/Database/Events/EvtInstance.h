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

/*! \brief Instance base event.
 * \ingroup Database
 */
class T_DLLCLASS EvtInstance : public IEvent
{
	T_RTTI_CLASS;

public:
	EvtInstance(const Guid& instanceGuid);

	/*! \brief Instance's guid.
	 *
	 * \return Instance's guid.
	 */
	const Guid& getInstanceGuid() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE;

private:
	Guid m_instanceGuid;
};

	}
}

#endif	// traktor_db_EvtInstance_H
