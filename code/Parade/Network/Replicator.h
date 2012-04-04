#ifndef traktor_parade_Replicator_H
#define traktor_parade_Replicator_H

#include <list>
#include <map>
#include "Core/Object.h"
#include "Core/RefArray.h"

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
class MessageRecorder;
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
			uint32_t peerId,
			const Object* eventObject
		) = 0;
	};

	Replicator();

	virtual ~Replicator();

	/*! \brief
	 */
	bool create(IReplicatorPeers* replicatorPeers, const ISerializable* joinParams);

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
	void sendEvent(uint32_t peerId, const ISerializable* eventObject);

	/*! \brief Broadcast high priority event to all peers.
	 */
	void broadcastEvent(const ISerializable* eventObject);

	/*! \brief
	 */
	uint32_t getMaxPeerId() const;

	/*! \brief Attach an object to a ghost peer.
	 *
	 * This permits attaching user objects to ghost
	 * peers at anytime.
	 */
	void setGhostObject(uint32_t peerId, Object* ghostObject);

	/*! \brief
	 */
	Object* getGhostObject(uint32_t peerId) const;

	/*! \brief Set ghost origin.
	 *
	 * Ghost origin is used to determine which frequency
	 * of transmission to use for each peer.
	 */
	void setGhostOrigin(uint32_t peerId, const Vector4& origin);

	/*! \brief Get state of ghost peer.
	 *
	 * The state of ghost peers are extrapolated
	 * in order to have a virtually identical
	 * state as the actual peer.
	 */
	Ref< const IReplicatableState > getGhostState(uint32_t peerId) const;

	/*! \brief Get network time.
	 */
	float getTime() const { return m_time; }

private:
	struct T_MATH_ALIGN16 GhostPeer
	{
		Vector4 origin;
		Ref< StateProphet > prophet;
		Ref< Object > object;
		float timeUntilTx;
		float lastTime;
		uint32_t packetCount;
		float latency;
	};

	struct Event
	{
		float time;
		uint32_t eventId;
		uint32_t peerId;
		Ref< const ISerializable > object;
	};

	std::vector< const TypeInfo* > m_eventTypes;
	Ref< MessageRecorder > m_recorder;
	Ref< IReplicatorPeers > m_replicatorPeers;
	Ref< const ISerializable > m_joinParams;
	RefArray< IListener > m_listeners;
	Vector4 m_origin;
	Ref< const IReplicatableState > m_state;
	std::map< uint32_t, GhostPeer* > m_ghostPeers;
	std::list< Event > m_eventsIn;
	std::list< Event > m_eventsOut;
	float m_time;

	void sendIAm(uint32_t peerId);
};

	}
}

#endif	// traktor_parade_Replicator_H
