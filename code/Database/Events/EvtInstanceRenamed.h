#ifndef traktor_db_EvtInstanceRenamed_H
#define traktor_db_EvtInstanceRenamed_H

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

/*! \brief Instance renamed event.
 * \ingroup Database
 */
class T_DLLCLASS EvtInstanceRenamed : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceRenamed();

	EvtInstanceRenamed(const Guid& instanceGuid, const std::wstring& previousName);

	/*! \brief Instance's previous name.
	 *
	 * \return Previous name of instance.
	 */
	const std::wstring& getPreviousName() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_previousName;
};

	}
}

#endif	// traktor_db_EvtInstanceRenamed_H
