/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Transform.h"
#include "Core/Timer/Timer.h"
#include "Jungle/INetworkTopology.h"
#include "Jungle/NetworkTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JUNGLE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

}

namespace traktor::jungle
{

class IReplicatorEventListener;
class IReplicatorStateListener;
class ReplicatorProxy;
class State;
class StateTemplate;

/*! Network replicator.
 * \ingroup Jungle
 *
 * Replicator manages a node network of peers and synchronization
 * of state between all nodes.
 *
 * Each peer is able to send data messages to any other peer.
 *
 * In order to make synchronization of state reliable
 * the replicator initially perform a time synchronization step
 * which tries to keep time as "equal" as possible between
 * all peers.
 */
class T_DLLCLASS Replicator
:	public Object
,	public INetworkTopology::INetworkCallback
{
	T_RTTI_CLASS;

public:
	struct Configuration
	{
		float timeVarianceThreshold = 0.03f;
		float nearDistance = 8.0f;
		float farDistance = 90.0f;
		float furthestDistance = 120.0f;
		float timeUntilTxStateNear = 0.1f;
		float timeUntilTxStateFar = 0.3f;
		float timeUntilTxPing = 1.0f;
	};

	virtual ~Replicator();

	/*! Create replicator.
	 *
	 * \param topology Network topology implementation.
	 * \param configuration Replicator configuration.
	 * \return True if successful.
	 */
	bool create(INetworkTopology* topology, const Configuration& configuration);

	/*! Destroy replicator. */
	void destroy();

	/*! Replace replicator configuration.
	 * 
	 * \param configuration Replicator configuration.
	 */
	void setConfiguration(const Configuration& configuration);

	/*! Get replicator's current configuration.
	 *
	 * \return Replicator current configuration.
	 */
	const Configuration& getConfiguration() const;

	/*! Remove all event types. */
	void removeAllEventTypes();

	/*! Register an event type.
	 *
	 * All peers must register same set of types in the
	 * same order.
	 *
	 * \param eventType Type of event.
	 */
	void addEventType(const TypeInfo& eventType);

	/*! Add replicator state listener.
	 *
	 * \param listener Replicator state listener.
	 */
	IReplicatorStateListener* addListener(IReplicatorStateListener* listener);

	/*! Remove replicator state listener.
	 *
	 * \param listener Replicator state listener.
	 */
	void removeListener(IReplicatorStateListener* listener);

	/*! Remove all replicator state listener. */
	void removeAllListeners();

	/*!
	 */
	IReplicatorEventListener* addEventListener(const TypeInfo& eventType, IReplicatorEventListener* eventListener);

	/*!
	 */
	void removeEventListener(IReplicatorEventListener* eventListener);

	/*!
	 */
	void removeAllEventListeners();

	/*! Update replicator.
	 *
	 * Perform all tasks required to maintain
	 * a synchronzied peer network.
	 *
	 * \return True if still connected.
	 */
	bool update();

	/*!
	 */
	void flush();

	/*! Get our name.
	 *
	 * \return Our name.
	 */
	const std::wstring& getName() const;

	/*! Set our status.
	 *
	 * \param New status.
	 */
	void setStatus(uint8_t status);

	/*! Get our status.
	 */
	uint8_t getStatus() const;

	/*! Check if caller is considered as primary peer.
	 *
	 * \return True if caller is primary peer.
	 */
	bool isPrimary() const;

	/*! Set our origin.
	 *
	 * Origin is used to determine which frequency
	 * of transmission to use to each peer.
	 *
	 * \param origin World origin of caller peer.
	 */
	void setOrigin(const Transform& origin);

	/*!
	 */
	void setStateTemplate(const StateTemplate* stateTemplate);

	/*! Set our replication state.
	 *
	 * Each peer have multiple ghost states which mirrors
	 * each peer real state.
	 *
	 * \param state Our current state.
	 */
	void setState(const State* state);

	/*! Get state.
	 */
	const State* getState() const;

	/*!
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

	/*! \
	 */
	double getBestLatency() const;

	/*! \
	 */
	double getBestReverseLatency() const;

	/*! \
	 */
	double getWorstLatency() const;

	/*! \
	 */
	double getWorstReverseLatency() const;

	/*! Broadcast high priority event to all peers.
	 */
	bool broadcastEvent(const ISerializable* eventObject, bool inOrder);

	/*! Send high priority event to primary peer.
	 */
	bool sendEventToPrimary(const ISerializable* eventObject, bool inOrder);

	/*! Set if time is automatically synchronized.
	 */
	void setTimeSynchronization(bool timeSynchronization);

	/*! Get network time.
	 */
	double getTime() const;

	/*! Get network time variance.
	 */
	double getTimeVariance() const;

	/*! Check if time synchronization is complete.
	 *
	 * \return True if time is synchronized.
	 */
	bool isTimeSynchronized() const;

private:
	friend class ReplicatorProxy;

	Ref< INetworkTopology > m_topology;
	Configuration m_configuration;
	std::vector< const TypeInfo* > m_eventTypes;
	RefArray< IReplicatorStateListener > m_listeners;
	SmallMap< const TypeInfo*, RefArray< IReplicatorEventListener > > m_eventListeners;
	std::wstring m_name;
	Timer m_timer;
	double m_time0 = 0.0;						/*!< Local time. */
	double m_timeContinuousSync = 0.0;			/*!< Network latency compensated time, but continuously synchronized. */
	double m_time = 0.0;						/*!< Network latency compensated time. */
	CircularVector< double, 32 > m_timeErrors;	/*!< History of time errors. */
	double m_timeVariance = 0.0;				/*!< Network time compensation time variance. */
	uint8_t m_status = 0;
	bool m_allowPrimaryRequests = true;
	Transform m_origin = Transform::identity();
	Ref< const StateTemplate > m_stateTemplate;
	Ref< const State > m_state;
	RefArray< ReplicatorProxy > m_proxies;
	bool m_sendState = false;
	bool m_timeSynchronization = true;
	bool m_timeSynchronized = false;
	uint32_t m_exceededDeltaTimeLimit = 0;

	std::wstring getLogPrefix() const;

	virtual bool nodeConnected(INetworkTopology* topology, net_handle_t node) override final;

	virtual bool nodeDisconnected(INetworkTopology* topology, net_handle_t node) override final;
};

}
