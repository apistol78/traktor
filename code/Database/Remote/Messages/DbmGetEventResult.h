#ifndef traktor_db_DbmGetEventResult_H
#define traktor_db_DbmGetEventResult_H

#include "Database/Remote/IMessage.h"
#include "Database/Types.h"
#include "Core/Guid.h"

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

/*! \brief Get event result.
 * \ingroup Database
 */
class T_DLLCLASS DbmGetEventResult : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmGetEventResult();

	DbmGetEventResult(ProviderEvent event, const Guid& eventId, bool remote);

	bool haveEvent() const { return m_haveEvent; }

	ProviderEvent getEvent() const { return ProviderEvent(m_event); }

	const Guid& getEventId() const { return m_eventId; }

	bool getRemote() const { return m_remote; }

	virtual bool serialize(ISerializer& s);

private:
	bool m_haveEvent;
	int32_t m_event;
	Guid m_eventId;
	bool m_remote;
};

	}
}

#endif	// traktor_db_DbmGetEventResult_H
