#ifndef traktor_net_Replicator_H
#define traktor_net_Replicator_H

#include <map>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Transform.h"
#include "Core/Timer/Timer.h"
#include "Net/Replication/INetworkTopology.h"
#include "Net/Replication/NetworkTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace net
	{

class ReplicatorProxy;
class State;
class StateTemplate;

class T_DLLCLASS Replicator
:	public Object
,	public INetworkTopology::INetworkCallback
{
	T_RTTI_CLASS;

public:
	class T_DLLCLASS IListener : public Object
	{
		T_RTTI_CLASS;

	public:
		enum
		{
			ReConnected = 1,
			ReDisconnected = 2,
			ReState = 3,
			ReStateError = 4,
			ReTimeAdjust = 5
		};

		virtual void notify(
			Replicator* replicator,
			float eventTime,
			uint32_t eventId,
			ReplicatorProxy* proxy,
			const Object* eventObject
		) = 0;
	};

	class T_DLLCLASS IEventListener : public Object
	{
		T_RTTI_CLASS;

	public:
		virtual void notify(
			Replicator* replicator,
			float eventTime,
			ReplicatorProxy* fromProxy,
			const Object* eventObject
		) = 0;
	};

	struct Configuration
	{
		float nearDistance;
		float farDistance;
		float furthestDistance;
		float timeUntilTxStateNear;
		float timeUntilTxStateFar;
		float timeUntilTxPing;

		Configuration()
		:	nearDistance(8.0f)
		,	farDistance(90.0f)
		,	furthestDistance(120.0f)
		,	timeUntilTxStateNear(0.1f)
		,	timeUntilTxStateFar(0.3f)
		,	timeUntilTxPing(1.0f)
		{
		}
	};

	Replicator();

	virtual ~Replicator();

	/*! \brief
	 */
	bool create(INetworkTopology* topology, const Configuration& configuration);

	/*! \brief
	 */
	void destroy();

	/*! \brief
	 */
	void setConfiguration(const Configuration& configuration);

	/*! \brief
	 */
	const Configuration& getConfiguration() const;

	/*! \brief
	 */
	void removeAllEventTypes();

	/*! \brief
	 */
	void addEventType(const TypeInfo& eventType);

	/*! \brief
	 */
	IListener* addListener(IListener* listener);

	/*! \brief
	 */
	void removeListener(IListener* listener);

	/*! \brief
	 */
	void removeAllListeners();

	/*! \brief
	 */
	IEventListener* addEventListener(const TypeInfo& eventType, IEventListener* eventListener);

	/*! \brief
	 */
	void removeEventListener(IEventListener* eventListener);

	/*! \brief
	 */
	void removeAllEventListeners();

	/*! \brief
	 *
	 * \return True if still connected.
	 */
	bool update();

	/*! \brief Get our name.
	 */
	const std::wstring& getName() const;

	/*! \brief Set our status.
	 */
	void setStatus(uint8_t status);

	/*! \brief Get our status.
	 */
	uint8_t getStatus() const;

	/*! \brief
	 */
	bool isPrimary() const;

	/*! \brief Set our origin.
	 *
	 * Origin is used to determine which frequency
	 * of transmission to use to each peer.
	 */
	void setOrigin(const Transform& origin);

	/*!
	 */
	void setStateTemplate(const StateTemplate* stateTemplate);

	/*! \brief Set our replication state.
	 *
	 * Each peer have multiple ghost states which mirrors
	 * each peer real state.
	 *
	 * \param state Our current state.
	 */
	void setState(const State* state);

	/*! \brief Get state.
	 */
	const State* getState() const;

	/*! \brief
	 */
	void setSendState(bool sendState);

	/*! \
	 */
	uint32_t getProxyCount() const;

	/*! \
	 */
	ReplicatorProxy* getProxy(uint32_t index) const;

	/*! \
	 */
	ReplicatorProxy* getPrimaryProxy() const;

	/*! \
	 */
	void resetAllLatencies();

	/*! \
	 */
	double getAverageLatency() const;

	/*! \
	 */
	double getAverageReverseLatency() const;

	/*! \brief Broadcast high priority event to all peers.
	 */
	bool broadcastEvent(const ISerializable* eventObject);

	/*! \brief Send high priority event to primary peer.
	 */
	bool sendEventToPrimary(const ISerializable* eventObject);

	/*! \brief Get network time.
	 */
	double getTime() const;

	/*! \brief Get network time variance.
	 */
	double getTimeVariance() const;

	/*! \brief Get time synchronization flag.
	 */
	bool isTimeSynchronized() const;

private:
	friend class ReplicatorProxy;

	Ref< INetworkTopology > m_topology;
	Configuration m_configuration;
	std::vector< const TypeInfo* > m_eventTypes;
	RefArray< IListener > m_listeners;
	std::map< const TypeInfo*, RefArray< IEventListener > > m_eventListeners;
	std::wstring m_name;
	Timer m_timer;
	double m_time0;								/*!< Local time. */
	double m_timeContinuousSync;				/*!< Network latency compensated time, but continuously synchronized. */
	double m_time;								/*!< Network latency compensated time. */
	CircularVector< double, 32 > m_timeErrors;	/*!< History of time errors. */
	double m_timeVariance;						/*!< Network time compensation time variance. */
	uint8_t m_status;
	bool m_allowPrimaryRequests;
	Transform m_origin;
	Ref< const StateTemplate > m_stateTemplate;
	Ref< const State > m_state;
	RefArray< ReplicatorProxy > m_proxies;
	bool m_sendState;
	bool m_timeSynchronized;

	std::wstring getLogPrefix() const;

	virtual bool nodeConnected(INetworkTopology* topology, net_handle_t node);

	virtual bool nodeDisconnected(INetworkTopology* topology, net_handle_t node);
};

	}
}

#endif	// traktor_net_Replicator_H
