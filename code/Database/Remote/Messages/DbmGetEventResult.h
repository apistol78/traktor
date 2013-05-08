#ifndef traktor_db_DbmGetEventResult_H
#define traktor_db_DbmGetEventResult_H

#include "Core/Guid.h"
#include "Database/Remote/IMessage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class IEvent;

/*! \brief Get event result.
 * \ingroup Database
 */
class T_DLLCLASS DbmGetEventResult : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmGetEventResult();

	DbmGetEventResult(const IEvent* event, bool remote);

	const IEvent* getEvent() const { return m_event; }

	bool getRemote() const { return m_remote; }

	virtual void serialize(ISerializer& s);

private:
	Ref< const IEvent > m_event;
	bool m_remote;
};

	}
}

#endif	// traktor_db_DbmGetEventResult_H
