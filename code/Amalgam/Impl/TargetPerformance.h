#ifndef traktor_amalgam_TargetPerformance_H
#define traktor_amalgam_TargetPerformance_H

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
	uint32_t memInUse;
	uint32_t heapObjects;

	TargetPerformance();

	virtual bool serialize(ISerializer& s);
};

	}
}

#endif	// traktor_amalgam_TargetPerformance_H
