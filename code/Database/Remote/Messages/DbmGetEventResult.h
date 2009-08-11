#ifndef traktor_db_DbmGetEventResult_H
#define traktor_db_DbmGetEventResult_H

#include "Database/Remote/IMessage.h"
#include "Database/Types.h"
#include "Core/Guid.h"

namespace traktor
{
	namespace db
	{

/*! \brief Get event result.
 * \ingroup Database
 */
class DbmGetEventResult : public IMessage
{
	T_RTTI_CLASS(DbmGetEventResult)

public:
	DbmGetEventResult();

	DbmGetEventResult(ProviderEvent event, const Guid& eventId, bool remote);

	bool haveEvent() const { return m_haveEvent; }

	ProviderEvent getEvent() const { return ProviderEvent(m_event); }

	const Guid& getEventId() const { return m_eventId; }

	bool getRemote() const { return m_remote; }

	virtual bool serialize(Serializer& s);

private:
	bool m_haveEvent;
	int32_t m_event;
	Guid m_eventId;
	bool m_remote;
};

	}
}

#endif	// traktor_db_DbmGetEventResult_H
