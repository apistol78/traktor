#ifndef traktor_parade_Replicator_H
#define traktor_parade_Replicator_H

#include <list>
#include <map>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Parade/Network/ReplicatorTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace parade
	{

class IReplicatableState;
class IReplicatorPeers;
class StateProphet;

class T_DLLCLASS Replicator : public Object
{
	T_RTTI_CLASS;

public:
	class IListener : public Object
	{
		T_RTTI_CLASS;

	public:
		enum
		{
			ReConnected = 1,
			ReDisconnected = 2,
			ReBroadcastEvent = 3,
			ReState = 4
		};

		virtual void notify(
			Replicator* replicator,
			float eventTime,
			uint32_t eventId,
			handle_t peerHandle,
			const Object* eventObject
		) = 0;
	};

	Replicator();

	virtual ~Replicator();

	/*! \brief
	 */
	bool create(IReplicatorPeers* replicatorPeers);

	/*! \brief
	 */
	void destroy();

	/*! \brief
	 */
	void addEventType(const TypeInfo& eventType);

	/*! \brief
	 */
	void addListener(IListener* listener);

	/*! \brief
	 */
	void update(float dT);

	/*! \brief Set our origin.
	 *
	 * Origin is used to determine which frequency
	 * of transmission to use to each peer.
	 */
	void setOrigin(const Vector4& origin);

	/*! \brief Set our replication state.
	 *
	 * Each peer have multiple ghost states which mirrors
	 * each peer real state.
	 */
	void setState(const IReplicatableState* state);

	/*! \brief Send high priority event to a single peer.
	 */
	void sendEvent(handle_t peerHandle, const ISerializable* eventObject);

	/*! \brief Broadcast high priority event to all peers.
	 */
	void broadcastEvent(const ISerializable* eventObject);

	/*! \brief
	 */
	bool isPrimary() const;

	/*! \brief
	 */
	uint32_t getPeerCount() const;

	/*! \brief
	 */
	handle_t getPeerHandle(uint32_t peerIndex) const;

	/*! \brief Get peer name.
	 */
	std::wstring getPeerName(handle_t peerHandle) const;

	/*! \brief Get peer average latency.
	 * \return Latency in milliseconds.
	 */
	int32_t getPeerLatency(handle_t peerHandle) const;

	/*! \brief Check if peer is connected.
	 */
	bool isPeerConnected(handle_t peerHandle) const;

	/*! \brief Attach an object to a ghost peer.
	 *
	 * This permits attaching user objects to ghost
	 * peers at anytime.
	 */
	void setGhostObject(handle_t peerHandle, Object* ghostObject);

	/*! \brief
	 */
	Object* getGhostObject(handle_t peerHandle) const;

	/*! \brief Set ghost origin.
	 *
	 * Ghost origin is used to determine which frequency
	 * of transmission to use for each peer.
	 */
	void setGhostOrigin(handle_t peerHandle, const Vector4& origin);

	/*! \brief Get state of ghost peer.
	 *
	 * The state of ghost peers are extrapolated
	 * in order to have a virtually identical
	 * state as the actual peer.
	 */
	Ref< const IReplicatableState > getGhostState(uint32_t peerHandle) const;

	/*! \brief Get network time.
	 */
	float getTime() const { return m_time; }

private:
	struct Event
	{
		float time;
		uint32_t eventId;
		handle_t handle;
		Ref< const ISerializable > object;
	};

	struct Ghost
	{
		Vector4 origin;
		Ref< StateProphet > prophet;
		Ref< Object > object;
	};

	struct Peer
	{
		bool established;
		bool disconnected;
		bool corrected;
		Ghost* ghost;
		float timeUntilTx;
		float lastTime;
		float latency;
		uint32_t packetCount;

		Peer()
		:	established(false)
		,	disconnected(false)
		,	corrected(false)
		,	ghost(0)
		,	timeUntilTx(0.0f)
		,	lastTime(0.0f)
		,	latency(0.0f)
		,	packetCount(0)
		{
		}
	};

	Guid m_id;
	std::vector< const TypeInfo* > m_eventTypes;
	Ref< IReplicatorPeers > m_replicatorPeers;
	RefArray< IListener > m_listeners;
	Vector4 m_origin;
	Ref< const IReplicatableState > m_state;
	std::map< handle_t, Peer > m_peers;
	std::list< Event > m_eventsIn;
	std::list< Event > m_eventsOut;
	float m_time;

	void sendIAm(handle_t peerHandle, uint8_t sequence, const Guid& id);

	void sendBye(handle_t peerHandle);
};

	}
}

#endif	// traktor_parade_Replicator_H
