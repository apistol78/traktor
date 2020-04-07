#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

/*! Target performance event from running target.
 * \ingroup Runtime
 */
class T_DLLCLASS TargetPerformance : public ISerializable
{
	T_RTTI_CLASS;

public:
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
	int32_t memCount;
	int32_t memDeltaCount;
	uint32_t heapObjects;
	uint32_t passCount;
	uint32_t drawCalls;
	uint32_t primitiveCount;
	uint32_t residentResourcesCount;
	uint32_t exclusiveResourcesCount;
	uint32_t bodyCount;
	uint32_t activeBodyCount;
	uint32_t manifoldCount;
	uint32_t queryCount;
	uint32_t activeSoundChannels;

	TargetPerformance();

	virtual void serialize(ISerializer& s) override final;
};

	}
}

