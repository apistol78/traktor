#ifndef traktor_parade_IReplicatableState_H
#define traktor_parade_IReplicatableState_H

#include "Core/Serialization/ISerializable.h"

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

class T_DLLCLASS IReplicatableState : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool verify(const IReplicatableState* targetState) const = 0;

	virtual Ref< IReplicatableState > extrapolate(const IReplicatableState* targetState, float T) const = 0;
};

	}
}

#endif	// traktor_parade_IReplicatableState_H
