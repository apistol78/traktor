#ifndef traktor_parade_StateProphet_H
#define traktor_parade_StateProphet_H

#include "Core/Object.h"
#include "Core/Containers/CircularVector.h"

namespace traktor
{
	namespace parade
	{

class IReplicatableState;

class StateProphet : public Object
{
	T_RTTI_CLASS;

public:
	void push(float T, IReplicatableState* state);

	Ref< const IReplicatableState > get(float T) const;

private:
	struct History
	{
		float T;
		Ref< IReplicatableState > state;
	};

	CircularVector< History, 16 > m_history;
};

	}
}

#endif	// traktor_parade_StateProphet_H
