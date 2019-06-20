#pragma once

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

	DbmGetEventResult(uint64_t sqnr, const IEvent* event, bool remote);

	uint64_t getSequenceNumber() const { return m_sqnr; }

	const IEvent* getEvent() const { return m_event; }

	bool getRemote() const { return m_remote; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint64_t m_sqnr;
	Ref< const IEvent > m_event;
	bool m_remote;
};

	}
}

