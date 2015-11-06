#ifndef traktor_amalgam_TargetPerformance_H
#define traktor_amalgam_TargetPerformance_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

/*! \brief Target performance event from running target.
 * \ingroup Amalgam
 */
class T_DLLCLASS TargetPerformance : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct FrameMarker
	{
		uint8_t id;
		uint8_t level;
		float begin;
		float end;

		FrameMarker()
		:	id(0)
		,	level(0)
		,	begin(0.0f)
		,	end(0.0)
		{
		}

		void serialize(ISerializer& s);
	};

	float time;
	float fps;
	float update;
	float build;
	float render;
	float physics;
	float input;
	float garbageCollect;
	float steps;
	float interval;
	uint32_t collisions;
	uint32_t memInUse;
	uint32_t memInUseScript;
	uint32_t heapObjects;
	uint32_t drawCalls;
	uint32_t primitiveCount;
	uint32_t residentResourcesCount;
	uint32_t exclusiveResourcesCount;
	uint32_t bodyCount;
	uint32_t activeBodyCount;
	uint32_t manifoldCount;
	uint32_t queryCount;
	uint32_t activeSoundChannels;
	AlignedVector< FrameMarker > frameMarkers;

	TargetPerformance();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_amalgam_TargetPerformance_H
