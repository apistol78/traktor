#ifndef traktor_editor_PipelineAgentsManager_H
#define traktor_editor_PipelineAgentsManager_H

#include <map>
#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;
class Thread;

	namespace db
	{

class ConnectionManager;
class ConnectionString;

	}

	namespace net
	{

class DiscoveryManager;
class StreamServer;

	}

	namespace editor
	{

class PipelineAgent;

/*! \brief
 * \ingroup Editor
 */
class T_DLLCLASS PipelineAgentsManager : public Object
{
	T_RTTI_CLASS;

public:
	PipelineAgentsManager(
		net::DiscoveryManager* discoveryManager,
		net::StreamServer* streamServer,
		db::ConnectionManager* dbConnectionManager
	);

	virtual ~PipelineAgentsManager();

	bool create(
		const PropertyGroup* settings,
		const db::ConnectionString& sourceDatabaseCs,
		const db::ConnectionString& outputDatabaseCs
	);

	void destroy();

	int32_t getAgentCount() const;

	PipelineAgent* getIdleAgent();

	int32_t getAgentIndex(const PipelineAgent* agent) const;

	void waitUntilAllIdle();

private:
	Ref< net::DiscoveryManager > m_discoveryManager;
	Ref< net::StreamServer > m_streamServer;
	Ref< db::ConnectionManager > m_dbConnectionManager;
	Guid m_sessionId;
	Ref< const PropertyGroup > m_settings;
	Thread* m_threadUpdate;
	mutable Semaphore m_lock;
	Event m_eventAgentsUpdated;
	std::map< std::wstring, Ref< PipelineAgent > > m_agents;

	void threadUpdate();
};

	}
}

#endif	// traktor_editor_PipelineAgentsManager_H
