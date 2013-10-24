#ifndef traktor_spray_SequenceInstance_H
#define traktor_spray_SequenceInstance_H

#include "Core/Object.h"
#include "Core/Math/Transform.h"
#include "Spray/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

struct Context;
class ITriggerInstance;

/*! \brief Sequence instance.
 * \ingroup Spray
 */
class T_DLLCLASS SequenceInstance : public Object
{
	T_RTTI_CLASS;

public:
	void update(Context& context, const Transform& transform, float T, bool enable);

private:
	friend class Sequence;

	struct Key
	{
		float T;
		Ref< ITriggerInstance > trigger;	
	};

	Ref< const Sequence > m_sequence;
	int32_t m_index;

	SequenceInstance(const Sequence* sequence);
};

	}
}

#endif	// traktor_spray_SequenceInstance_H
