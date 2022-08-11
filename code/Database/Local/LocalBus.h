#pragma once

#include "Database/Provider/IProviderBus.h"
#include "Core/Guid.h"

namespace traktor
{

class ISharedMemory;

	namespace db
	{

/*! Local database event bus.
 * \ingroup Database
 *
 * Using named mutex to listen for new events across multiple
 * processes on the same machine.
 */
class LocalBus : public IProviderBus
{
	T_RTTI_CLASS;

public:
	explicit LocalBus(const std::wstring& journalFileName);

	virtual ~LocalBus();

	void close();

	virtual bool putEvent(const IEvent* event) override final;

	virtual bool getEvent(uint64_t& inoutSqnr, Ref< const IEvent >& outEvent, bool& outRemote) override final;

private:
	Guid m_localGuid;
	std::wstring m_journalFileName;
	Ref< ISharedMemory > m_shm;
};

	}
}

