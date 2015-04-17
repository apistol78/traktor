#ifndef traktor_net_ReplicatorProxy_H
#define traktor_net_ReplicatorProxy_H

#include <list>
#include "Core/Object.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Transform.h"
#include "Net/Replication/NetworkTypes.h"
#include "Net/Replication/ReplicatorTypes.h"

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

class IEventListener;
class INetworkTopology;
class IReplicatorEventListener;
class State;
class StateTemplate;

class T_DLLCLASS ReplicatorProxy : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief
	 */
	net_handle_t getHandle() const;

	/*! \brief
	 */
	const std::wstring& getName() const;

	/*! \brief
	 */
	Object* getUser() const;

	/*! \brief
	 */
	uint8_t getStatus() const;

	/*! \brief
	 */
	bool isLatencyReliable() const;

	/*! \brief
	 */
	double getLatency() const;

	/*! \brief
	 */
	double getLatencySpread() const;

	/*! \brief
	 */
	double getReverseLatency() const;

	/*! \brief
	 */
	double getReverseLatencySpread() const;

	/*! \brief
	 */
	void resetLatencies();

	/*! \brief
	 */
	double getTimeRate() const;

	/*! \brief
	 */
	bool isConnected() const;

	/*! \brief
	 */
	bool setPrimary();

	/*! \brief
	 */
	bool isPrimary() const;

	/*! \brief
	 */
	bool isRelayed() const;

	/*! \brief
	 */
	void setObject(Object* object);

	/*! \brief
	 */
	Object* getObject() const;

	/*! \brief
	 */
	void setOrigin(const Transform& origin);

	/*! \brief
	 */
	const Transform& getOrigin() const;

	/*! \brief
	 */
	void setStateTemplate(const StateTemplate* stateTemplate);

	/*! \brief
	 */
	const StateTemplate* getStateTemplate() const;

	/*! \brief
	 */
	Ref< const State > getState(double time, double limit) const;

	/*! \brief
	 */
	Ref< const State > getFilteredState(double time, double limit, const State* currentState, float filterCoeff) const;

	/*! \brief
	 */
	void resetStates();

	/*! \brief
	 */
	void setSendState(bool sendState);

	/*! \brief Send high priority event to this ghost.
	 */
	void sendEvent(const ISerializable* eventObject);

private:
	friend class Replicator;

	struct Event
	{
		double time;
		uint32_t size;
		uint32_t count;
		RMessage msg;
	};

	struct EventSlot
	{
		uint32_t time;
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

	uint8_t m_sequence;
	std::list< Event > m_unacknowledgedEvents;
	CircularVector< std::pair< uint32_t, uint8_t >, 128 > m_lastEvents;
	uint8_t m_dispatchEvent;
	EventSlot m_eventSlots[256];

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

	int32_t updateEventQueue();

	bool receivedEventAcknowledge(const ReplicatorProxy* from, uint8_t sequence);

	bool enqueueEvent(uint32_t time, uint8_t sequence, const ISerializable* eventObject);

	bool dispatchEvents(const SmallMap< const TypeInfo*, RefArray< IReplicatorEventListener > >& eventListeners);

	void updateLatency(double localTime, double remoteTime, double roundTrip, double latencyReverse, double latencyReverseSpread);

	bool receivedState(double localTime, double stateTime, const void* stateData, uint32_t stateDataSize);

	void disconnect();

	ReplicatorProxy(Replicator* replicator, net_handle_t handle, const std::wstring& name, Object* user);
};

	}
}

#endif	// traktor_net_ReplicatorProxy_H
