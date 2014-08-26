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

class INetworkTopology;
class State;
class StateTemplate;

class T_DLLCLASS ReplicatorProxy : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief
	 */
	const std::wstring& getName() const;

	/*! \brief
	 */
	uint8_t getStatus() const;

	/*! \brief
	 */
	double getLatency() const;

	/*! \brief
	 */
	double getLatencyUp() const;

	/*! \brief
	 */
	double getLatencyDown() const;

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
	Ref< const State > getState(double timeOffset) const;

	/*! \brief Send high priority event to this ghost.
	 */
	void sendEvent(const ISerializable* eventObject);

private:
	friend class Replicator;

	struct Event
	{
		double time;
		uint32_t size;
		RMessage msg;
	};

	Replicator* m_replicator;
	net_handle_t m_handle;
	std::wstring m_name;
	uint8_t m_status;
	Ref< Object > m_object;
	Transform m_origin;
	Ref< const StateTemplate > m_stateTemplate;
	Ref< const State > m_stateN2;
	double m_stateTimeN2;
	Ref< const State > m_stateN1;
	double m_stateTimeN1;
	Ref< const State > m_state0;
	double m_stateTime0;
	uint8_t m_sequence;
	std::list< Event > m_events;
	CircularVector< uint8_t, 32 > m_lastEvents;
	float m_distance;
	double m_timeUntilTxPing;
	double m_timeUntilTxState;
	CircularVector< double, 17 > m_roundTrips;
	double m_latencyMedian;
	double m_latencyReverse;

	bool updateEventQueue();

	bool isEventNew(uint8_t sequence);

	void receivedEventAcknowledge(uint8_t sequence);

	void updateLatency(double roundTrip, double reverseLatency);

	bool receivedState(double stateTime, const void* stateData, uint32_t stateDataSize);

	void disconnect();

	ReplicatorProxy(Replicator* replicator, net_handle_t handle, const std::wstring& name);
};

	}
}

#endif	// traktor_net_ReplicatorProxy_H
