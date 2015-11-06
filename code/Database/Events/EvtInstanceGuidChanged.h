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

/*! \brief Instance guid changed event.
 * \ingroup Database
 */
class T_DLLCLASS EvtInstanceGuidChanged : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceGuidChanged();

	EvtInstanceGuidChanged(const Guid& instanceGuid, const Guid& instancePreviousGuid);

	/*! \brief Instance's previous guid.
	 *
	 * \return Previous guid of instance.
	 */
	const Guid& getInstancePreviousGuid() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Guid m_instancePreviousGuid;
};

	}
}

#endif	// traktor_db_EvtInstanceGuidChanged_H
