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

/*! Put bus event.
 * \ingroup Database
 */
class T_DLLCLASS DbmPutEvent : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmPutEvent(uint32_t handle = 0, const IEvent* event = 0);

	uint32_t getHandle() const { return m_handle; }

	const IEvent* getEvent() const { return m_event; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_handle;
	Ref< const IEvent > m_event;
};

	}
}

