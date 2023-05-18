/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Object.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Transform.h"
#include "Jungle/NetworkTypes.h"
#include "Jungle/ReplicatorTypes.h"

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

class IEventListener;
class INetworkTopology;
class IReplicatorEventListener;
class State;
class StateTemplate;

class T_DLLCLASS ReplicatorProxy : public Object
{
	T_RTTI_CLASS;

public:
	/*!
	 */
	net_handle_t getHandle() const;

	/*!
	 */
	const std::wstring& getName() const;

	/*!
	 */
	Object* getUser() const;

	/*!
	 */
	uint8_t getStatus() const;

	/*!
	 */
	bool isLatencyReliable() const;

	/*!
	 */
	double getLatency() const;

	/*!
	 */
	double getLatencySpread() const;

	/*!
	 */
	double getReverseLatency() const;

	/*!
	 */
	double getReverseLatencySpread() const;

	/*!
	 */
	void resetLatencies();

	/*!
	 */
	double getTimeRate() const;

	/*!
	 */
	bool isConnected() const;

	/*!
	 */
	bool setPrimary();

	/*!
	 */
	bool isPrimary() const;

	/*!
	 */
	bool isRelayed() const;

	/*!
	 */
	void setObject(Object* object);

	/*!
	 */
	Object* getObject() const;

	/*!
	 */
	void setOrigin(const Transform& origin);

	/*!
	 */
	const Transform& getOrigin() const;

	/*!
	 */
	void setStateTemplate(const StateTemplate* stateTemplate);

	/*!
	 */
	const StateTemplate* getStateTemplate() const;

	/*!
	 */
	Ref< const State > getState(double time, double limit) const;

	/*!
	 */
	Ref< const State > getFilteredState(double time, double limit, const State* currentState, float filterCoeff) const;

	/*!
	 */
	void resetStates();

	/*!
	 */
	void setSendState(bool sendState);

	/*! Send high priority event to this ghost.
	 */
	void sendEvent(const ISerializable* eventObject, bool inOrder);

private:
	friend class Replicator;

	struct TxEvent
	{
		double time;
		uint32_t size;
		uint32_t count;
		RMessage msg;
	};

	struct RxEvent
	{
		uint32_t time;
		uint8_t sequence;
		Ref< const ISerializable > eventObject;
	};

	Replicator* m_replicator;
	net_handle_t m_handle;

	/*! \group Proxy information. */
	//@{

	std::wstring m_name;
	Ref< Object > m_user;
	uint8_t m_status;
	Ref< Object > m_object;
	Transform m_origin;
	float m_distance;
	bool m_sendState;
	bool m_issueStateListeners;

	//@}

	/*! \group Shadow states. */
	//@{

	Ref< const StateTemplate > m_stateTemplate;
	Ref< const State > m_stateN2;
	double m_stateTimeN2;
	Ref< const State > m_stateN1;
	double m_stateTimeN1;
	Ref< const State > m_state0;
	double m_stateTime0;
	double m_stateReceivedTime;

	//@}

	/*! \group Event management. */
	//@{

	uint8_t m_txSequence;
	uint8_t m_txSequenceInOrder;
	std::list< TxEvent > m_txEvents;

	RxEvent m_rxEventsInOrderQueue[256];
	uint8_t m_rxEventsInOrderSequence;
	AlignedVector< RxEvent > m_rxEvents;
	CircularVector< uint8_t, 64 > m_rxEventSequences[2];

	//@}

	/*! \group Time measurements. */
	//@{

	double m_timeUntilTxPing;
	double m_timeUntilTxState;
	CircularVector< std::pair< double, double >, 33 > m_remoteTimes;
	CircularVector< double, 33 > m_roundTrips;
	double m_timeRate;
	double m_latency;
	double m_latencyStandardDeviation;
	double m_latencyReverse;
	double m_latencyReverseStandardDeviation;
	
	// @}

	int32_t updateTxEventQueue();

	bool receivedTxEventAcknowledge(const ReplicatorProxy* from, uint8_t sequence, bool inOrder);

	bool receivedRxEvent(uint32_t time, uint8_t sequence, const ISerializable* eventObject, bool inOrder);

	bool dispatchRxEvents(const SmallMap< const TypeInfo*, RefArray< IReplicatorEventListener > >& eventListeners);

	void updateLatency(double localTime, double remoteTime, double roundTrip, double latencyReverse, double latencyReverseSpread);

	bool receivedState(double localTime, double stateTime, const void* stateData, uint32_t stateDataSize);

	void disconnect();

	std::wstring getLogIdentifier() const;

	ReplicatorProxy(Replicator* replicator, net_handle_t handle, const std::wstring& name, Object* user);
};

}
